// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "AgoraLaneMinion.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Engine/Public/TimerManager.h"
#include "Engine/World.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "AgoraMinionMovementComponent.h"
#include "Agora.h"
#include "AgoraLaneWaypoint.h"
#include "AgoraLaneMinionAIController.h"
#include "AgoraTowerBase.h"
#include "TeamInterface.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraGameplayTags.h"

DECLARE_CYCLE_STAT(TEXT("OverlapTargetAcquisition"), STAT_OverlapTargetAcquisiton, STATGROUP_AgoraAI);
DECLARE_CYCLE_STAT(TEXT("EndOverlapTargetAcquisition"), STAT_EndOverlapTargetAcquisiton, STATGROUP_AgoraAI);
DECLARE_CYCLE_STAT(TEXT("UpdateTarget"), STAT_UpdateTarget, STATGROUP_AgoraAI);
DECLARE_CYCLE_STAT(TEXT("DetermineTarget"), STAT_DetermineTarget, STATGROUP_AgoraAI);
DECLARE_CYCLE_STAT(TEXT("DetermineNextWaypoint"), STAT_DetermineNextWaypoint, STATGROUP_AgoraAI);


AAgoraLaneMinion::AAgoraLaneMinion(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer.SetDefaultSubobjectClass<UAgoraMinionMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	TargetAcquisitionRadius = CreateDefaultSubobject<USphereComponent>(TEXT("TargetAquisitionRadius"));
	TargetAcquisitionRadius->SetupAttachment(RootComponent);
	TargetAcquisitionRadius->SetCollisionProfileName("OverlapOnlyPawn");
}

void AAgoraLaneMinion::BeginPlay()
{
    Super::BeginPlay();

    
	if (DuskTeamMesh && DawnTeamMesh) 
    {
		if (GetTeam() == ETeam::Dawn) 
        {
			GetMesh()->SetSkeletalMesh(DawnTeamMesh);
		}
		else 
        {
			GetMesh()->SetSkeletalMesh(DuskTeamMesh);
		}
	}

    if (!HasAuthority())
    {
        TargetAcquisitionRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        TargetAcquisitionRadius->SetGenerateOverlapEvents(false);
        return;
    }

	TargetAcquisitionRadius->OnComponentBeginOverlap.AddDynamic(this, &AAgoraLaneMinion::NotifyActorEnterTargetAcquisitionRadius);
	TargetAcquisitionRadius->OnComponentEndOverlap.AddDynamic(this, &AAgoraLaneMinion::NotifyActorExitTargetAcquisitionRadius);

    // When this minion is spawned, we need to check to see if its overlapping anything
    // if this isn't called, the initial 
    TargetAcquisitionRadius->UpdateOverlaps();
}

void AAgoraLaneMinion::NotifyActorEnterTargetAcquisitionRadius(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor == this) { return; }
    
    SCOPE_CYCLE_COUNTER(STAT_OverlapTargetAcquisiton);

    IDamageReceiverInterface* OverlappedAsDamageReceiverInterface = Cast<IDamageReceiverInterface>(OtherActor);

    if (UAgoraGameplayTags::IsHero(OtherActor))
    {
        if (UAgoraBlueprintFunctionLibrary::IsFriendly(this, OtherActor))
        {
            TrackedAllies.AddUnique(OtherActor);
            //TRACE(AgoraAIVerbose, Log, "%s entered acquisition radius of %s, listening for damage.", *GetNameSafe(OtherActor), *GetNameSafe(this));
            if (OverlappedAsDamageReceiverInterface)
            {
                OverlappedAsDamageReceiverInterface->GetDamageReceivedDelegate().AddUniqueDynamic(this, &AAgoraLaneMinion::NotifyNearbyHeroDamaged);

            }
        }
        else
        {
            TrackedEnemies.AddUnique(OtherActor);
        }
    }
    else if (UAgoraGameplayTags::IsMinion(OtherActor) && !UAgoraGameplayTags::IsJungleCreep(OtherActor))
    {
        if (UAgoraBlueprintFunctionLibrary::IsFriendly(this, OtherActor))
        {
            TrackedAllies.AddUnique(OtherActor);
        }
        else
        {
            TrackedEnemies.AddUnique(OtherActor);
        }
    }
    else if (UAgoraGameplayTags::IsTower(OtherActor))
    {
        if (UAgoraBlueprintFunctionLibrary::IsFriendly(this, OtherActor))
        {
            TrackedAllies.AddUnique(OtherActor);
        }
        else
        {
            TrackedEnemies.AddUnique(OtherActor);
        }
    }
}


void AAgoraLaneMinion::NotifyActorExitTargetAcquisitionRadius(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

    SCOPE_CYCLE_COUNTER(STAT_EndOverlapTargetAcquisiton);

    IGameplayTagAssetInterface* OverlappedAsTagAssetInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
    IDamageReceiverInterface* OverlappedAsDamageReceiverInterface = Cast<IDamageReceiverInterface>(OtherActor);

    if (!OverlappedAsTagAssetInterface) { return; }

    if (UAgoraBlueprintFunctionLibrary::IsFriendly(this, OtherActor))
    {
        if (OverlappedAsTagAssetInterface->HasMatchingGameplayTag(UAgoraBlueprintFunctionLibrary::GetGameplayTag("Descriptor.Unit.Hero")))
        {
            //TRACE(AgoraAIVerbose, Log, "%s left acquisition radius of %s", *GetNameSafe(OtherActor), *GetNameSafe(this));
            if (OverlappedAsDamageReceiverInterface)
            {
                OverlappedAsDamageReceiverInterface->GetDamageReceivedDelegate().RemoveDynamic(this, &AAgoraLaneMinion::NotifyNearbyHeroDamaged);
            }
        }
        TrackedAllies.Remove(OtherActor);
    }
    else
    {
        TrackedEnemies.Remove(OtherActor);
    }
}



void AAgoraLaneMinion::NotifyNearbyHeroDamaged(float CurrentHealth, float MaxHealth, const FDamageContext& DamageContext)
{
    // however we want to react to this in c++
    // ...

    // however we want to react to this in BP
    ReceiveNearbyHeroDamaged(CurrentHealth, MaxHealth, DamageContext);
}

void AAgoraLaneMinion::SetGameplayTagTarget(AActor* NewTarget, float Duration, bool bUpdateMinionTarget)
{
    if (!GameplayTagTarget)
    {
        GameplayTagTarget = NewTarget;
        AAIController* AICon = Cast<AAIController>(GetController());
        if (AICon && AICon->GetBlackboardComponent())
        {
            AICon->GetBlackboardComponent()->SetValueAsObject("GameplayTagTarget", NewTarget);
        }
        GetWorld()->GetTimerManager().SetTimer(GameplayTagTargetReset, this, &AAgoraLaneMinion::ClearGameplayTagTarget, Duration, false, -1.0f);
    }
    if (bUpdateMinionTarget)
    {
        UpdateCurrentTarget();
    }
}

void AAgoraLaneMinion::ClearGameplayTagTarget()
{
    GameplayTagTarget = nullptr;
    AAIController* AICon = Cast<AAIController>(GetController());
    if (AICon && AICon->GetBlackboardComponent())
    {
        AICon->GetBlackboardComponent()->SetValueAsObject("GameplayTagTarget", nullptr);
    }
    GetWorldTimerManager().ClearTimer(GameplayTagTargetReset);

}

void AAgoraLaneMinion::SetCallForHelpTarget(AActor* NewTarget, float Duration, bool bUpdateMinionTarget)
{
    if (!CallForHelpTarget)
    {
        CallForHelpTarget = NewTarget;
        AAIController* AICon = Cast<AAIController>(GetController());
        if (AICon && AICon->GetBlackboardComponent())
        {
            AICon->GetBlackboardComponent()->SetValueAsObject("CallForHelpTarget", NewTarget);
        }
        GetWorld()->GetTimerManager().SetTimer(CallForHelpTargetReset, this, &AAgoraLaneMinion::ClearCallForHelpTarget, Duration, false, -1.0f);
    }
    if (bUpdateMinionTarget)
    {
        UpdateCurrentTarget();
    }
}

void AAgoraLaneMinion::ClearCallForHelpTarget()
{
    CallForHelpTarget = nullptr;
    AAIController* AICon = Cast<AAIController>(GetController());
    if (AICon && AICon->GetBlackboardComponent())
    {
        AICon->GetBlackboardComponent()->SetValueAsObject("CallForHelpTarget", nullptr);
    }
    GetWorldTimerManager().ClearTimer(CallForHelpTargetReset);

}

AActor* AAgoraLaneMinion::UpdateCurrentTarget()
{
    AActor* UpdatedTarget = nullptr;

    SCOPE_CYCLE_COUNTER(STAT_UpdateTarget);

    if (GameplayTagTarget)
    {
        UpdatedTarget = GameplayTagTarget;
    }
    else if (CallForHelpTarget)
    {
        UpdatedTarget = CallForHelpTarget;
    }
    else 
    {
        UpdatedTarget = DetermineTargetInAcquisitionRadius();
        if (!UpdatedTarget)
        {
            UpdatedTarget = UpdateTargetWaypoint();
        }
    }

    if (UpdatedTarget != Target)
    {
        //TRACE(AgoraAIVerbose, Log, "%s changed target to %s", *GetNameSafe(this), *GetNameSafe(UpdatedTarget));
    }
    Target = UpdatedTarget;
    return Target;
}

AActor* AAgoraLaneMinion::DetermineTargetInAcquisitionRadius()
{

    SCOPE_CYCLE_COUNTER(STAT_DetermineTarget);

    if (!TargetAcquisitionRadius) { return nullptr; }

    AActor* FoundTarget = nullptr;
    uint8 PriorityIndex = 254;

    // gross, will be removed for an action list eventually
    for (AActor* PotentialTarget : TrackedEnemies)
    {
        IGameplayTagAssetInterface* PotentialTargetAsTagAssetInterface = Cast<IGameplayTagAssetInterface>(PotentialTarget);

        if (!PotentialTargetAsTagAssetInterface) { continue; }

        for (int32 i = 0; i < AggroRadiusTagPriority.Num(); i++)
        {
            if (PotentialTargetAsTagAssetInterface->HasMatchingGameplayTag(AggroRadiusTagPriority[i]) && PriorityIndex >= i)
            {
                if (PriorityIndex == i)
                {
                    if (GetDistanceTo(PotentialTarget) < GetDistanceTo(FoundTarget))
                    {
                        FoundTarget = PotentialTarget;
                    }
                }
                else
                {
                    FoundTarget = PotentialTarget;
                    PriorityIndex = i;
                }
            }
        }
    }

    return FoundTarget;
}

int32 AAgoraLaneMinion::DetermineNearestWaypointIndex()
{

    if (Waypoints.Num() == 0) 
    { 
        //TRACE(AgoraAI, Warning, "%s attempted to choose a waypoint, however there were no waypoints to choose from.", *GetNameSafe(this));  
        return -1;
    }

    int32 ChosenWaypointIndex = 0;
    for(int32 i = 0; i < Waypoints.Num(); i++)
    {
        if(GetDistanceTo(Waypoints[i]) < GetDistanceTo(Waypoints[ChosenWaypointIndex]))
        {
            ChosenWaypointIndex = i;
        }
    }
    return ChosenWaypointIndex;
}

AAgoraLaneWaypoint* AAgoraLaneMinion::UpdateTargetWaypoint()
{
    if (!Waypoints.IsValidIndex(TargetWaypointIndex)) { return nullptr; }

    SCOPE_CYCLE_COUNTER(STAT_DetermineNextWaypoint);

    // If we're close to the target waypoint, just retarget to the next in line
    if (Waypoints[TargetWaypointIndex] && GetDistanceTo(Waypoints[TargetWaypointIndex]) <= WAYPOINT_ACCEPTANCE_RADIUS)
    {
        // only wana go next if there is a next
        if (Waypoints.IsValidIndex(TargetWaypointIndex + 1))
        {
            //TRACE(AgoraAIVerbose, Log, "Going Next %s", *GetNameSafe(Waypoints[TargetWaypointIndex + 1]));
            TargetWaypointIndex += 1;
        }
    }
    else if (!Waypoints[TargetWaypointIndex] || ShouldRecalculateTargetWaypoint()) // otherwise, if we are way off track then we should completely recalculate
    {
        int32 ClosestWaypointIndex = DetermineNearestWaypointIndex();
        if (ClosestWaypointIndex != -1 && Waypoints.IsValidIndex(ClosestWaypointIndex + 1))
        {
            // if the closest waypoint is actually behind us, we don't want to turn around and go back
            // thus if we've already progressed towards the waypoint beyond the closest, we'll set our target to that instead
            float DistanceBetweenWaypoints = Waypoints[ClosestWaypointIndex]->GetDistanceTo(Waypoints[ClosestWaypointIndex + 1]);
            float DistanceToNextWaypoint = GetDistanceTo(Waypoints[ClosestWaypointIndex + 1]);
            if (DistanceBetweenWaypoints > DistanceToNextWaypoint)
            {
                TargetWaypointIndex = ClosestWaypointIndex + 1;
            }
            else
            {
                // sometimes the closest waypoint is actually the one we want though
                TargetWaypointIndex = ClosestWaypointIndex;
            }
        }
        else
        {
            // we're either at the end of the line or we couldnt find a waypoint
            TargetWaypointIndex = ClosestWaypointIndex;
        }
    }

    return Waypoints[TargetWaypointIndex];
}

bool AAgoraLaneMinion::ShouldRecalculateTargetWaypoint()
{
    // if we are at the start/end of the waypoints, or if there is only one waypoint, or if we are at the last waypoint
    if (TargetWaypointIndex == 0 || Waypoints.Num() <= 1 || TargetWaypointIndex == Waypoints.Num() - 1) { return false; }

    AAgoraLaneWaypoint* CurrentTargetWaypoint = Waypoints[TargetWaypointIndex];
    AAgoraLaneWaypoint* PreviousTargetWaypoint = Waypoints[TargetWaypointIndex-1];

    // if we are close to the previous waypoint, then we can assume we just changed waypoints and shouldn't recalculate
    if (GetDistanceTo(PreviousTargetWaypoint) <= WAYPOINT_ACCEPTANCE_RADIUS)
    {
        return false;
    }
    else if (GetDistanceTo(CurrentTargetWaypoint) > PreviousTargetWaypoint->GetDistanceTo(CurrentTargetWaypoint))
    {
        return true;
    }
    return false;
}