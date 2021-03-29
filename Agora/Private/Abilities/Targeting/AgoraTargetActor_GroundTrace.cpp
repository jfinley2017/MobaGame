// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraTargetActor_GroundTrace.h"
#include "Agora.h"
#include "Gameframework/Actor.h"
#include "AIController.h"
#include "Gameframework/PlayerController.h"
#include "GameplayAbilityTargetActor.h"
#include "Abilities/GameplayAbility.h"
#include "AgoraTargetingReticle.h"
#include <DrawDebugHelpers.h>
#include <Kismet/KismetMathLibrary.h>

AAgoraTargetActor_GroundTrace::AAgoraTargetActor_GroundTrace(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer) {
	
	SweepParams.TeamRule = ESweepTeamRule::NoRule;
}

FHitResult AAgoraTargetActor_GroundTrace::PerformTrace(AActor* InSourceActor)
{
	AAIController* AI = GetAIController(InSourceActor);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGameplayAbilityTargetActor_GroundTrace), false);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(InSourceActor);
	
	FHitResult ReturnHitResult;
	FVector TraceCenter = GetTraceCenter();

	if (AI)
	{
		AActor* Targeted = AI->GetFocusActor();
		
		if (!Targeted)
		{
			// We don't have an AI target, so just give up.
			TRACE(AgoraAI, Warning, "AI tried to use AgoraTargetActor_GroundTrace without a focus target");
			CancelTargeting();
		}

		// Find the actor, then find the floor
		TraceDown(ReturnHitResult, Targeted->GetActorLocation(), Params);
	}
	else
	{
		FThirdPersonTrace Trace = FThirdPersonTrace(OwningAbility->GetCurrentActorInfo()->PlayerController.Get(), InSourceActor, TraceCenter, MaxRange);

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(InSourceActor);
		if (!Trace.Trace(ReturnHitResult, SweepParams, CollisionParams, ECC_Damage))
		{
			TraceDown(ReturnHitResult, ReturnHitResult.Location, Params);
		}

#if ENABLE_DRAW_DEBUG
		float DebugDuration = CVarDebugTargetingTraces.GetValueOnGameThread();
		if (DebugDuration != 0.0f)
		{
			DrawDebugLine(GetWorld(), TraceCenter, ReturnHitResult.Location, FColor::Yellow, false);
		}
#endif

		if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
		{
			LocalReticleActor->SetActorLocation(ReturnHitResult.Location);
			LocalReticleActor->SetActorRotation(Trace.CameraDirection.ToOrientationRotator());
		}
	}


	// Reset the trace start so the target data uses the correct origin
	ReturnHitResult.TraceStart = TraceCenter;

	return ReturnHitResult;
}

void AAgoraTargetActor_GroundTrace::InitializeReticle()
{
	AAgoraTargetingReticle* AgoraReticle = Cast<AAgoraTargetingReticle>(ReticleActor);
	if (AgoraReticle)
	{
		AgoraReticle->InitializeRadius(EffectRadius);
	}

	// This does need to go after..which is a bit weird perhaps
	Super::InitializeReticle();
}

bool AAgoraTargetActor_GroundTrace::ShouldProduceTargetData() const
{
	return IsAI() ? true : Super::ShouldProduceTargetData();
}

bool AAgoraTargetActor_GroundTrace::IsConfirmTargetingAllowed()
{
	return bIsValidTarget;
}

void AAgoraTargetActor_GroundTrace::TraceDown(FHitResult& OutHitResult, FVector TraceFrom, FCollisionQueryParams Params)
{
	FVector Backwards = (TraceFrom - GetTraceCenter()).GetSafeNormal();
	TraceFrom = TraceFrom - Backwards;		//Pull back 1 unit to avoid scraping down walls;
	FVector TraceEnd = TraceFrom;
	TraceEnd.Z -= 99999.0f;

	SourceActor->GetWorld()->LineTraceSingleByChannel(OutHitResult, TraceFrom, TraceEnd, ECollisionChannel::ECC_Visibility, Params);
	//LineTraceWithFilter(OutHitResult, TraceFrom, TraceEnd, Params);

	// Recursively step backwards until the trace works
	if (!IsLocationInRange(OutHitResult.Location))
	{
		// We could make this configurable buut it's more likely to be confusing
		// This could go poorly for abilities with very long max ranges, but generally you're not interested in a 
		// cross map tesla dome or something of that sort
		FVector NewFrom = TraceFrom - (Backwards * TraceBackStepSize);

#if ENABLE_DRAW_DEBUG
		float DebugDuration = CVarDebugTargetingTraces.GetValueOnGameThread();
		if (DebugDuration != 0.0f)
		{
			DrawDebugLine(GetWorld(), TraceFrom, GetTraceCenter(), FColor::Red);
			DrawDebugSphere(GetWorld(), NewFrom, 50, 12, FColor::Red);
		}
#endif

		FVector NewDirectionFromStart = (NewFrom - GetTraceCenter()).GetSafeNormal();
		// If our backwards direction changed, this means we've gone past our start location. We can't back up more than this, so invalidate
		if (!(Backwards - NewDirectionFromStart).IsNearlyZero())
		{
			SetIsValidTarget(false);
			TRACE(Agora, Warning, "Trace tried to backup past the original trace start, aborting to prevent infinite loop");
			return;
		}

		return TraceDown(OutHitResult, NewFrom, Params);
	}

	SetIsValidTarget(true);

#if ENABLE_DRAW_DEBUG
	float DebugDuration = CVarDebugTargetingTraces.GetValueOnGameThread();
	if (DebugDuration != 0.0f && OutHitResult.IsValidBlockingHit())
	{
		DrawDebugLine(GetWorld(), TraceFrom, OutHitResult.Location, FColor::Green, false);
	}
#endif
}

void AAgoraTargetActor_GroundTrace::SetIsValidTarget(bool bIsValid)
{
	bIsValidTarget = bIsValid;
	if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
	{
		LocalReticleActor->SetIsTargetValid(bIsValid);
	}
}
