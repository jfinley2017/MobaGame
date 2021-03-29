// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#pragma once

#include "CoreMinimal.h"
#include "AgoraMinionBase.h"
#include "AgoraLaneMinion.generated.h"

/****************************************************************************************************************/
/*                                             AgoraLaneMinions                                                 */
/* - Base class for lane minions.                                                                               */   
/* - Concerns:                                                                                                  */
/* - 1.) At some point we're going to need to think about pooling this class, as it is spawned frequently       */
/*       and spawning is an expensive action. This means that rather than having BP_LaneMinionSiege,            */
/*       BP_LaneMinionMelee, BP_LaneMinionRanged, BP_LaneMinionSuper, we'll need to provide the differences     */
/*       in some sort of Activate(FLaneMinionParams) function which is called by the spawner when it wants      */
/*       to spawn a minion. A method like this would also make it easy to change the looks of the minion types, */ 
/*       since they would all be designated in one spot.                                                        */
/*       Alternatively, we could pool different numbers of each BP... but if we're setting teams and waypoints  */    
/*       on spawn we might as well go the full way.                                                             */
/****************************************************************************************************************/

// How far away from a waypoint do we have to be in order for us to progress to the next waypoint
#define WAYPOINT_ACCEPTANCE_RADIUS 1000.0f

DECLARE_STATS_GROUP(TEXT("AgoraLaneMinion"), STATGROUP_AgoraAI, STATCAT_Advanced);

class AAgoraLaneWaypoint;
class USphereComponent;
class UBasicAttackComponent;

UCLASS()
class AGORA_API AAgoraLaneMinion : public AAgoraMinionBase
{
	GENERATED_BODY()

public:

	AAgoraLaneMinion(const FObjectInitializer& ObjectInitializer);

    // List of all waypoints that are relevant to this minion
    // I'd rather this be a linked list, as its easier to wrap your head around (and produces cleaner logic for finding next/previous waypoints)
    // I'd also rather not have this public but some other files are just pulling from this value already and i dont wana check out even more files atm
    // @TODO ^
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Agora|LaneMinion")
    TArray<AAgoraLaneWaypoint*> Waypoints;

    // Forces the minion to update its current target.
    // Goes through this list:
    // GameplayTagTarget
    // CallForHelpTarget
    // DetermineTargetInAcquisitionRadius()
    // UpdateTargetWaypoint()
    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    AActor* UpdateCurrentTarget();

protected:

    // ACharacter interface
    virtual void BeginPlay() override;
    // ~ACharacter interface

    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    void SetCallForHelpTarget(AActor* NewTarget, float Duration, bool bUpdateMinionTarget);

    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    void ClearCallForHelpTarget();

    // Calculates the "passive" target, whatever the top priority is in our acquisition range.
   // This function is O(TrackedEnemies.Num())
    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    AActor* DetermineTargetInAcquisitionRadius();

    // Calculates and sets a new target waypoint. Returns what was set (will be equal to the value of TargetWaypoint)
    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    AAgoraLaneWaypoint* UpdateTargetWaypoint();

    // Used to determine whether or not we're on track, if this returns false then we have to assume
    // that we need to completely recalculate which waypoint is next.
    // Helps gracefully recover from situations where we were leashed into the jungle/leashed too far up/down lane
    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    bool ShouldRecalculateTargetWaypoint();

    // Iterates through the waypoint array, returning whichever is closest to us. 
    // This function is O(Waypoints.Num())
    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    int32 DetermineNearestWaypointIndex();

    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    void SetGameplayTagTarget(AActor* NewTarget, float Duration, bool bUpdateMinionTarget);

    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    void ClearGameplayTagTarget();

    // Events to help with book keeping and the binding of relevant events
    UFUNCTION()
    void NotifyActorEnterTargetAcquisitionRadius(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
    UFUNCTION()
    void NotifyActorExitTargetAcquisitionRadius(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Listeners, typically bound to so that a minion can respond to friendly targets being damaged
    UFUNCTION()
    void NotifyNearbyHeroDamaged(float CurrentHealth, float MaxHealth, const FDamageContext& DamageContext);
    
    // How far this minion can "sense". Tracked Enemies/Allies exist in this sphere
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AgoraMinionLane")
    USphereComponent* TargetAcquisitionRadius;

    /** The main skeletal for team Dawn */
    UPROPERTY(EditDefaultsOnly, Category = "Agora|LaneMinion")
    USkeletalMesh* DawnTeamMesh;

    /** The main skeletal for team Dusk */
    UPROPERTY(EditDefaultsOnly, Category = "Agora|LaneMinion")
    USkeletalMesh* DuskTeamMesh;

    // The tags which we are looking for in our aggro radius. 
    // Lower index means a higher priority
    UPROPERTY(EditDefaultsOnly, Category = "Agora|LaneMinion")
    TArray<FGameplayTag> AggroRadiusTagPriority;

    // The current index of the waypoint in Waypoints that we are progressing towards
    UPROPERTY(BlueprintReadWrite, Category = "Agora|LaneMinion")
    uint8 TargetWaypointIndex = 0;

    // The allies that we are currently aware of
    UPROPERTY(BlueprintReadWrite, Category = "AgoraLaneMinion")
    TArray<AActor*> TrackedAllies;

    // The enemies that we are currently aware of
    UPROPERTY(BlueprintReadWrite, Category = "AgoraLaneMinion")
    TArray<AActor*> TrackedEnemies;

    // Call for help instigator target. Typically the result of NearbyHeroDamaged being invoked
    UPROPERTY(BlueprintReadOnly, Category = "AgoraMinionLane")
    AActor* CallForHelpTarget = nullptr;

    // Top priority plugin target. Typically used for items/mechanics similar to "Pack Leader" from old Paragon
    // Currently there is no way to have 'teirs' of these. eg its impossible to make one mechanic have priority over another
    UPROPERTY(BlueprintReadOnly, Category = "AgoraMinionLane")
    AActor* GameplayTagTarget = nullptr;

    FTimerHandle GameplayTagTargetReset;
    FTimerHandle CallForHelpTargetReset;

    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraMinionLane")
    void ReceiveNearbyHeroDamaged(float CurrentHealth, float MaxHealth, const FDamageContext& DamageContext);

   
};
