#include "GATargetActorRadius.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "WorldCollision.h"
#include "Components/CapsuleComponent.h"

#include "AgoraCharacterBase.h"
#include "Abilities/GameplayAbility.h"
#include "Agora.h"
#include "Abilities/Targeting/AgoraTargetingReticle.h"

AGATargetActorRadius::AGATargetActorRadius()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	ShouldProduceTargetDataOnServer = true;
}

void AGATargetActorRadius::StartTargeting(UGameplayAbility* InAbility)
{
	Super::StartTargeting(InAbility);
	SourceActor = InAbility->GetCurrentActorInfo()->AvatarActor.Get();

	//this bit was mysteriously left out of the built-in radial GATargetActor....
	//why?
	if (ReticleClass)
	{
		AGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle>(ReticleClass, GetActorLocation(), GetActorRotation());
		if (SpawnedReticleActor)
		{
			AAgoraTargetingReticle* AgoraReticle = Cast<AAgoraTargetingReticle>(SpawnedReticleActor);
			if (AgoraReticle)
			{
				AgoraReticle->InitializeRadius(Radius);
			}

			SpawnedReticleActor->InitializeReticle(this, MasterPC, ReticleParams);
			

			ReticleActor = SpawnedReticleActor;

			// This is to catch cases of playing on a listen server where we are using a replicated reticle actor.
			// (In a client controlled player, this would only run on the client and therefor never replicate. If it runs
			// on a listen server, the reticle actor may replicate. We want consistancy between client/listen server players.
			// Just saying 'make the reticle actor non replicated' isnt a good answer, since we want to mix and match reticle
			// actors and there may be other targeting types that want to replicate the same reticle actor class).
			if (!ShouldProduceTargetDataOnServer)
			{
				SpawnedReticleActor->SetReplicates(false);
			}
		}
	}
}

void AGATargetActorRadius::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (SourceActor)
	{
		FVector Origin = StartLocation.GetTargetingTransform().GetLocation();
		FGameplayAbilityTargetDataHandle Handle = MakeTargetData(PerformOverlap(Origin), Origin);
		TargetDataReadyDelegate.Broadcast(Handle);
	}
}

FGameplayAbilityTargetDataHandle AGATargetActorRadius::MakeTargetData(const TArray<TWeakObjectPtr<AActor>>& Actors, const FVector& Origin) const
{
	if (OwningAbility)
	{
		/** Use the source location instead of the literal origin */
		return StartLocation.MakeTargetDataHandleFromActors(Actors, false);
	}

	return FGameplayAbilityTargetDataHandle();
}

TArray<TWeakObjectPtr<AActor> >	AGATargetActorRadius::PerformOverlap(const FVector& Origin)
{
	bool bTraceComplex = false;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RadiusTargetingOverlap), bTraceComplex);
	Params.bReturnPhysicalMaterial = false;

	TArray<FOverlapResult> Overlaps;

	SourceActor->GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(Radius), Params);

	TArray<TWeakObjectPtr<AActor>>	HitActors;

	for (int32 i = 0; i < Overlaps.Num(); ++i)
	{
		//Should this check to see if these pawns are in the AimTarget list?
		APawn* PawnActor = Cast<APawn>(Overlaps[i].GetActor());
		if (PawnActor && !HitActors.Contains(PawnActor) && Filter.FilterPassesForActor(PawnActor))
		{
			HitActors.Add(PawnActor);
		}
	}

	return HitActors;
}
void AGATargetActorRadius::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
	{
        AAgoraCharacterBase* Character = Cast<AAgoraCharacterBase>(MasterPC->GetPawn());
		LocalReticleActor->SetIsTargetValid(true);
		float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		FVector ReticleLocation = MasterPC->GetPawn()->GetActorLocation();
		ReticleLocation.Z = ReticleLocation.Z - CapsuleHalfHeight + 5.f;
		LocalReticleActor->SetActorLocation(ReticleLocation);
	}
}

void AGATargetActorRadius::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ReticleActor.IsValid())
	{
		ReticleActor.Get()->Destroy();
	}
	Super::EndPlay(EndPlayReason);
}