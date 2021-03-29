// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "AgoraEntityLibrary.h"
#include "AgoraSweepLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AgoraSweep, Log, All);
DECLARE_STATS_GROUP(TEXT("AgoraSweep"), STATGROUP_AgoraSweep, STATCAT_Advanced);

USTRUCT(BlueprintType)
struct FMeleeHitResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "MeleeHitResult")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "MeleeHitResult")
    TArray<AActor*> OverlappingTargets;
};

UENUM(BlueprintType)
enum class ESweepTeamRule : uint8
{
    NoRule, // returns allies and enemies
    Enemies, // returns enemies only
    Allies // returns allies only

};

UENUM(BlueprintType)
enum class ESweepTagRule : uint8
{
    Any, // Overlap actor can have any of the supplied tags
    All // Overlapped actor must have all the supplied tags
};

USTRUCT(BlueprintType)
struct FAgoraSweepTagContainer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "AgoraTagContainer")
    ESweepTagRule TagQueryRule;

    UPROPERTY(BlueprintReadWrite, Category = "AgoraTagContainer")
    FGameplayTagContainer Tags;
    
};

USTRUCT(BlueprintType)
struct FAgoraSweepQueryParams
{
    GENERATED_BODY()

    /** Requires actors to be Friends/Enemies/Either, in relation to the sweeping Actor. */
    UPROPERTY(BlueprintReadWrite, Category = "AgoraSweepQueryParams")
    ESweepTeamRule TeamRule = ESweepTeamRule::NoRule;

    /** Requires actors to have Any/All of these tags, as per the RequiredTagsRule. */
    UPROPERTY(BlueprintReadWrite, Category = "AgoraSweepQueryParams")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(BlueprintReadWrite, Category = "AgoraSweepQueryParams")
    ESweepTagRule RequiredTagsRule = ESweepTagRule::Any;

    /** Ignores actors with Any/All of these tags, as per the RestrictedTagsRule. */
    UPROPERTY(BlueprintReadWrite, Category = "AgoraSweepQueryParams")
    FGameplayTagContainer RestrictedTags;

    UPROPERTY(BlueprintReadWrite, Category = "AgoraSweepQueryParams")
    ESweepTagRule RestrictedTagsRule = ESweepTagRule::Any;

    bool MatchesQuery(const AActor* SweepingActor, const AActor* SweptActor) const
    {
        return SweepingActor && SweptActor && PassesTeamCheck(SweepingActor, SweptActor) && PassesTagCheck(SweptActor);
    }

protected:

    /** Returns true if the relationship between the SweepingActor and the Swept actor is as specified in the TeamRule */
    bool PassesTeamCheck(const AActor* SweepingActor, const AActor* SweptActor) const;

    /** Returns true if the has Any/All of the tags specified by Required tags and does not have Any/All of the tags specified by RestrictedTags */
    bool PassesTagCheck(const AActor* SweptActor) const;

};

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraSweepLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    /** Wraps an OverlapMultiByObject type to provide additional filtering/checking. */
    static bool AgoraSweepMulti(UObject* WorldContextObject, AActor* SweepingActor, 
                                const FAgoraSweepQueryParams& AgoraQueryParams, TArray<AActor*>& OutHitActors, 
                                const FVector& SweepStart, const FQuat& SweepRot, 
                                const FCollisionObjectQueryParams& ObjectQueryParams, const FCollisionShape& InShape,
                                const FCollisionQueryParams& Params = FCollisionQueryParams::DefaultQueryParam);

    // @TODO Joe - move me into ability class
    UFUNCTION(BlueprintPure, Category = "AgoraSweepLibrary")
    static FMeleeHitResult GenerateMeleeHitResult_Cleave(AActor* PerformingActor, FVector HitBox, 
                                                         float MaxRange, bool bDebug);

    /**
    * Performs an AgoraSweepMulti in a sphere around @SweepLocation
    * Filters for team relationships to @SweepingActor.
    * @Param SweepingActor The actor performing the sweep
    * @Param AgoraQueryParams Should we ignore Enemies? Friendlies?
    * @Param SweepSphereRadius The radius extending from @SweepStartLoc
    * @Param SweepLocation The location at which we are sweeping
    * @Param OutOverlappingActors Filled found actors that pass additional filtering
    * @Param IgnoredActors Actors which we should not consider
    * @Return true if the sweep did not fail.
    */
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "AgoraSweepLibrary")
    static bool AgoraSweepMultiSphere(UObject* WorldContextObject, AActor* SweepingActor, 
                                      const FAgoraSweepQueryParams& AgoraQueryParams, float SweepSphereRadius, 
                                      const FVector& SweepLocation, TArray<AActor*>& OutOverlappingActors, 
                                      const TArray<AActor*>& IgnoredActors);

    /**
    * Performs an AgoraSweepMulti in a box around @SweepLocation
    * Filters for team relationships to @SweepingActor.
    * @Param SweepingActor The actor performing the Sweep
    * @Param AgoraQueryParams Should we ignore Enemies? Friendlies?
    * @Param SweepBoxExtent The extent of the sweeping box.
    * @Param SweepLocation The location at which we are sweeping
    * @Param SweepRotation The rotation at which are we sweeping
    * @Param OutOverlappingActors Filled found actors that pass additional filtering
    * @Param IgnoredActors Actors which we should not consider
    * @Return true if the sweep did not fail.
    */
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "AgoraSweepLibrary")
    static bool AgoraSweepMultiBox(UObject* WorldContextObject, AActor* SweepingActor, 
                                   const FAgoraSweepQueryParams& AgoraQueryParams, const FVector& SweepBoxExtent, 
                                   const FVector& SweepLocation, const FRotator& SweepRotation,
                                   TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors);

    /**
    * Performs an AgoraSweepMulti in a capsule around @SweepLocation
    * Filters for team relationships to @SweepingActor.
    * @Param SweepingActor The actor performing the Sweep
    * @Param AgoraQueryParams Should we ignore Enemies? Friendlies?
    * @Param SweepCapsuleRadius The radius of the sweeping capsule.
    * @Param SweepCapsuleHalfHeight The half-height of the sweeping capsule.
    * @Param SweepLocation The location at which we are sweeping
    * @Param SweepRotation The rotation at which are we sweeping
    * @Param OutOverlappingActors Filled found actors that pass additional filtering
    * @Param IgnoredActors Actors which we should not consider
    * @Return true if the sweep did not fail.
    */
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "AgoraSweepLibrary")
    static bool AgoraSweepMultiCapsule(UObject* WorldContextObject, AActor* SweepingActor, 
                                       const FAgoraSweepQueryParams& AgoraQueryParams, const float SweepCapsuleRadius, 
                                       const float SweepCapsuleHalfHeight, const FVector& SweepLocation, 
                                       const FRotator& SweepRotation, TArray<AActor*>& OutOverlappingActors, 
                                       const TArray<AActor*>& IgnoredActors);


    /**
    * This sweep is not like the others!
    * This function randomly shotguns line traces from the specified origin onto a rectangle located at (PyramidOrigin + PyramidHeight) with the specified dimensions.
    * Useful for things like Murdock's buckshot, where a pyramidal collision check is ideal.
    * Uses a caller-controllable bivariate normal distribution to randomly distribute the line traces. Defaults to an evenly distributed spread of points.
    * @Param SweepingActor The actor performing the Sweep
    * @Param AgoraQueryParams Should we ignore Enemies? Friendlies?
    * @Param BaseWidth Width of the base of the pyramid
    * @Param BaseHeight Height of the base of the pyramid
    * @Param PyramidOrigin The location of the "tip" of the pyramid
    * @Param PyramidHeight The height of the pyramid from tip to base.
    * @Param PyramidRotation The rotation of the pyramid.
    * @Param OutOverlappingActors Filled found actors that pass additional filtering
    * @Param IgnoredActors Actors which we should not consider
    * @Param Spread Two values ranging from [1.0, 0.0) describing the spread of the traces. X value controls the spread along the width-axis. Y value controls the spread along the height-axis. E.g. A spread of (1.0, 1.0) means the traces are evenly distributed across the base. A spread of (0.1, 0.1) will make the traces concentrated more in the center.
    */
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "AgoraSweepLibrary")
    static bool AgoraSweepMultiPyramid(UObject* WorldContextObject, AActor* SweepingActor, 
                                       const FAgoraSweepQueryParams& AgoraQueryParams, const float BaseWidth, 
                                       const float BaseHeight, const FVector& PyramidOrigin, 
                                       const float PyramidHeight, const FRotator& PyramidRotation, 
                                       TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors, 
                                       const FVector2D& Spread = FVector2D(1.0f, 1.0f), bool bDebug = false);



    UFUNCTION(BlueprintPure, Category = "AgoraSweepLibrary")
    static FAgoraSweepQueryParams MakeSweepParamsForEnemies(FAgoraSweepTagContainer RequiredTags, FAgoraSweepTagContainer RestrictedTags);
        
    UFUNCTION(BlueprintPure, Category = "AgoraSweepLibrary")
    static FAgoraSweepQueryParams MakeSweepParamsForAllies(FAgoraSweepTagContainer RequiredTags, FAgoraSweepTagContainer RestrictedTags);
};
