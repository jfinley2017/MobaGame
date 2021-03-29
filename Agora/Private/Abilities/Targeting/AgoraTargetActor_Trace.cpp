// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraTargetActor_Trace.h"
#include "Agora.h"
#include <GameplayAbility.h>
#include <GameFramework/PlayerController.h>
#include <DrawDebugHelpers.h>
#include <AIController.h>

AAgoraTargetActor_Trace::AAgoraTargetActor_Trace(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

	SweepParams.TeamRule = ESweepTeamRule::Enemies;

	MaxRange = 999999.0f;

	// Without this SetActorLocationAndRotation will not work in Tick
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TargetRootComp"));
}

AAIController* AAgoraTargetActor_Trace::GetAIController(AActor* InSourceActor) const
{
	APawn* Pawn = Cast<APawn>(InSourceActor);
	if (Pawn)
	{
		AAIController* AI = Pawn->GetController<AAIController>();
		return AI;
	}
	return nullptr;
}

bool AAgoraTargetActor_Trace::IsAI() const
{
	AAIController* AI = GetAIController(SourceActor);
	return AI ? true : false;
}

FVector AAgoraTargetActor_Trace::GetTraceCenter() const
{
	return StartLocation.GetTargetingTransform().GetLocation();
}

// This method copies some of the code from the original trace actor
void AAgoraTargetActor_Trace::StartTargeting(UGameplayAbility* Ability)
{
	OwningAbility = Ability;
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

	if (ReticleClass)
	{
		AGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle>(ReticleClass, GetActorLocation(), GetActorRotation());
		if (SpawnedReticleActor)
		{
			// This is to catch cases of playing on a listen server where we are using a replicated reticle actor.
			// (In a client controlled player, this would only run on the client and therefor never replicate. If it runs
			// on a listen server, the reticle actor may replicate. We want consistancy between client/listen server players.
			// Just saying 'make the reticle actor non replicated' isnt a good answer, since we want to mix and match reticle
			// actors and there may be other targeting types that want to replicate the same reticle actor class).
			if (!ShouldProduceTargetDataOnServer)
			{
				SpawnedReticleActor->SetReplicates(false);
			}

			// This is the main difference
			ReticleActor = SpawnedReticleActor;
			InitializeReticle();
		}
	}
}

void AAgoraTargetActor_Trace::InitializeReticle()
{
	ReticleActor->InitializeReticle(this, MasterPC, ReticleParams);
}

void AAgoraTargetActor_Trace::Tick(float DeltaSeconds)
{
	if (SourceActor)
	{
		FVector StartPoint = GetTraceCenter();
		FHitResult Result = PerformTrace(SourceActor);

		FRotator Rot = SourceActor->GetActorRotation();
		if (!IsAI())
		{
			APlayerController* PC = OwningAbility->GetCurrentActorInfo()->PlayerController.Get();
			check(PC);

			FVector ViewStart;
			PC->GetPlayerViewPoint(ViewStart, Rot);
		}

		SetActorLocationAndRotation(Result.Location, Rot);
	}
}

bool AAgoraTargetActor_Trace::ShouldProduceTargetData() const
{
	if (IsAI())
	{
		return true;
	}

	return Super::ShouldProduceTargetData();
}

void AAgoraTargetActor_Trace::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ReticleActor.IsValid())
	{
		ReticleActor.Get()->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

void AAgoraTargetActor_Trace::ConfirmTargetingAndContinue()
{
	if (SourceActor)
	{
		FGameplayAbilityTargetDataHandle Handle = MakeTargetData(PerformTrace(SourceActor));
		TargetDataReadyDelegate.Broadcast(Handle);
	}
}

FHitResult AAgoraTargetActor_Trace::PerformTrace(AActor* InSourceActor)
{
	FHitResult ReturnHitResult;

	// Caring about AI here is pretty weird, but haven't come up with a better solution yet
	if (IsAI())
	{
		AAIController* AI = GetAIController(InSourceActor);
		AActor* Targeted = AI->GetFocusActor();
		
		if (!Targeted)
		{
			// We don't have an AI target, so just give up.
			TRACE(AgoraAI, Warning, "AI tried to use AgoraTargetActor_GroundTrace without a focus target");
			CancelTargeting();
		}

		ReturnHitResult.bBlockingHit = true;
		ReturnHitResult.Location = Targeted->GetActorLocation();
	}
	else
	{
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(InSourceActor);
		CollisionQueryParams.bIgnoreBlocks = bIgnoreBlocks;

		FThirdPersonTrace Trace = FThirdPersonTrace(OwningAbility->GetCurrentActorInfo()->PlayerController.Get(), InSourceActor, GetTraceCenter(), MaxRange);
		Trace.Trace(ReturnHitResult, SweepParams, CollisionQueryParams, ECC_Damage);

#if ENABLE_DRAW_DEBUG
		float DebugDuration = CVarDebugTargetingTraces.GetValueOnGameThread();

		if (DebugDuration != 0.0f)
		{
			DrawDebugDirectionalArrow(InSourceActor->GetWorld(), Trace.GetTraceCenter(), ReturnHitResult.Location, 25.0f, FColor::Orange, false, DebugDuration);
			DrawDebugSphere(InSourceActor->GetWorld(), ReturnHitResult.Location, 25.0f, 12, FColor::Orange, false, DebugDuration);
		}
#endif
	}

	if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
	{
		LocalReticleActor->SetActorLocation(ReturnHitResult.Location);
	}

	// Reset the trace start so the target data uses the correct origin
	ReturnHitResult.TraceStart = GetTraceCenter();

	return ReturnHitResult;
}

FGameplayAbilityTargetDataHandle AAgoraTargetActor_Trace::MakeTargetData(const FHitResult& HitResult) const
{
	/** Note: This will be cleaned up by the FGameplayAbilityTargetDataHandle (via an internal TSharedPtr) */
	return StartLocation.MakeTargetDataHandleFromHitResult(OwningAbility, HitResult);
}

FThirdPersonTrace::FThirdPersonTrace(const AController* PC, const AActor* SourceActor, const FVector& TraceCenter, float MaxRange)
	: SourceActor(SourceActor), TraceCenter(TraceCenter), MaxRange(MaxRange)
{
	ensureMsgf(PC, TEXT("FThirdPersonTrace doesnt know how to handle null PlayerController"));
	ensureMsgf(SourceActor, TEXT("FThirdPersonTrace doesnt know how to handle null SourceActor"));

	FRotator Rot;
	PC->GetPlayerViewPoint(CameraLocation, Rot);
	CameraDirection = Rot.Vector();

	WorldContext = PC->GetWorld();
}

FThirdPersonTrace::FThirdPersonTrace(const AActor* SourceActor, const FVector& CameraLocation, const FVector& CameraDirection, const FVector& TraceCenter, float MaxRange)
	: SourceActor(SourceActor), CameraLocation(CameraLocation), CameraDirection(CameraDirection), TraceCenter(TraceCenter), MaxRange(MaxRange)
{
	WorldContext = SourceActor->GetWorld();
}

FVector FThirdPersonTrace::GetTraceCenter() const
{
	return TraceCenter;
}

bool AAgoraTargetActor_Trace::IsLocationInRange(const FVector& Location) const
{
	return FVector::DistSquared(GetTraceCenter(), Location) <= (MaxRange * MaxRange);
}

bool FThirdPersonTrace::Trace(FHitResult& OutHitResult)
{
	FAgoraSweepQueryParams SweepParams;
	SweepParams.TeamRule = ESweepTeamRule::Enemies;
	FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(FThirdPersonTrace), false);

	CollisionParams.AddIgnoredActor(SourceActor);

	return Trace(OutHitResult, SweepParams, CollisionParams);
}

bool FThirdPersonTrace::Trace(FHitResult& OutHitResult, const FAgoraSweepQueryParams& SweepParams, const FCollisionQueryParams& CollisionParams, ECollisionChannel CollisionChannel)
{
	TArray<FHitResult> HitResults;

	// Don't start tracing until we're even with our trace center
	FVector TraceStart = CameraLocation + (CameraDistanceToCenterProjected() * CameraDirection);
	FVector TraceEnd;

	TraceMaxEnd(TraceEnd);

	WorldContext->LineTraceMultiByChannel(HitResults, TraceStart, TraceEnd, CollisionChannel, CollisionParams);

#if ENABLE_DRAW_DEBUG
	float DebugDuration = CVarDebugTargetingTraces.GetValueOnGameThread();

	if (DebugDuration != 0.0f)
	{
		DrawDebugDirectionalArrow(WorldContext, CameraLocation, TraceCenter, 10.0f, FColor::White, false, -1.0f);
		DrawDebugDirectionalArrow(WorldContext, CameraLocation, TraceStart, 10.0f, FColor::White, false, -1.0f);
		DrawDebugDirectionalArrow(WorldContext, TraceStart, TraceEnd, 10.0f, FColor::Black, false, -1.0f);
	}
#endif

	// hit results are sorted, closest first
	for (int32 HitIdx = 0; HitIdx < HitResults.Num(); ++HitIdx)
	{
		const FHitResult& Hit = HitResults[HitIdx];
		FVector ActorToHit = TraceCenter - Hit.Location;
		FVector TraceDirection = (TraceStart - TraceEnd).GetSafeNormal();


		// What if a tag rule is specified, but no team rule?
		// That's undefined behavior, if you get this ensure, figure it out
		//ensure(SweepParams.TeamRule != ESweepTeamRule::NoRule || (SweepParams.RequiredTags.Num() == 0 && SweepParams.RestrictedTags.Num() == 0));
		if (Hit.bBlockingHit)
		{
			OutHitResult = Hit;
			return true;
		}

		// If we specify a team rule, the hit actor needs to implement team interface
		bool bSkipQuery = SweepParams.TeamRule == ESweepTeamRule::NoRule; //) || Cast<ITeamInterface>(Hit.Actor.Get());
		bool bQueryResult = bSkipQuery || !Cast<ITeamInterface>(Hit.Actor.Get()) || SweepParams.MatchesQuery(SourceActor, Hit.Actor.Get());
		if (bQueryResult && !Hit.bStartPenetrating)
		{
			OutHitResult = Hit;
			OutHitResult.bBlockingHit = true;
			return false;
		}
	}

	OutHitResult.Location = TraceEnd;
	return false;
}

float FThirdPersonTrace::CameraDistanceToCenterProjected()
{
	return FVector::DotProduct(TraceCenter - CameraLocation, CameraDirection);
}

bool FThirdPersonTrace::TraceMaxEnd(FVector& OutMaxTrace)
{
	float CameraDotCenter = CameraDistanceToCenterProjected();
	if (CameraDotCenter >= 0)		//If this fails, we're pointed away from the center, but we might be inside the sphere and able to find a good exit point.
	{
		// The difference between how far the camera is from center, and how far along the camera trace that distance is
		float DistanceSquared = (TraceCenter - CameraLocation).SizeSquared() - (CameraDotCenter * CameraDotCenter);
		float RadiusSquared = (MaxRange * MaxRange);
		if (DistanceSquared <= RadiusSquared)
		{
			// See distance formula
			float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceAlongRay = CameraDotCenter + DistanceFromCamera;				//Subtracting instead of adding will get the other intersection point
			OutMaxTrace = CameraLocation + (DistanceAlongRay * CameraDirection);		//Cam aim point clipped to range sphere
			return true;
		}
	}

	return false;
}
