// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraSweepLibrary.h"

#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "Kismet/KismetMathLibrary.h"

#include "Agora.h"
#include "AgoraTeamLibrary.h"
#include "AgoraUtilityLibrary.h"


DEFINE_LOG_CATEGORY(AgoraSweep)

bool UAgoraSweepLibrary::AgoraSweepMulti(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, TArray<AActor*>& OutHitActors, const FVector& SweepStart, const FQuat& SweepRot, const FCollisionObjectQueryParams& ObjectQueryParams, const FCollisionShape& InShape, const FCollisionQueryParams& Params /*= FCollisionQueryParams::DefaultQueryParam*/)
{
    if (!WorldContextObject || !WorldContextObject->ImplementsGetWorld()) { return false; }

    TArray<FOverlapResult> OverlapResults;

    bool Success = WorldContextObject->GetWorld()->OverlapMultiByObjectType(OverlapResults, SweepStart, SweepRot, ObjectQueryParams, InShape, Params);
    if (Success)
    {
        for (FOverlapResult Result : OverlapResults)
        {
            if (AgoraQueryParams.MatchesQuery(SweepingActor, Result.GetActor()))
            {
                OutHitActors.Add(Result.GetActor());
            }
        }
    }

    return Success;
}

FMeleeHitResult UAgoraSweepLibrary::GenerateMeleeHitResult_Cleave(AActor* PerformingActor, FVector HitBox, float MaxRange, bool bDebug)
{
    if (!PerformingActor)
    {
        TRACESTATIC(AgoraSweep, Warning, "Could not generate melee hit result, PerformingActor is null");
        return FMeleeHitResult();
    }

    FRotator LookDirection = FRotator::ZeroRotator;
    FVector LookLocation = FVector::ZeroVector; // We don't really care about this value
    PerformingActor->GetActorEyesViewPoint(LookLocation, LookDirection);

    //line trace for the main target
    FHitResult LineTraceResult;
    FCollisionQueryParams LineTraceParams;
    LineTraceParams.AddIgnoredActor(PerformingActor);
    FVector TraceStart = FVector(PerformingActor->GetActorLocation().X, PerformingActor->GetActorLocation().Y, PerformingActor->GetActorLocation().Z + 75);
    FVector TraceEnd = TraceStart + LookDirection.Vector() * (MaxRange);
    PerformingActor->GetWorld()->LineTraceSingleByChannel(LineTraceResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, LineTraceParams);
    if (bDebug)
    {
        DrawDebugLine(PerformingActor->GetWorld(), TraceStart, TraceEnd, FColor::Red, true, 15.0f);
    }

    TArray<AActor*> OutOverlappingActors;
    FVector SweepStartLocation = TraceStart + LookDirection.Vector() * (MaxRange - HitBox.X / 2); //we want the box's far side to meet the end of the line trace.
    FVector SweepEndLocation = SweepStartLocation;

    FAgoraSweepQueryParams SweepQueryParams;
    SweepQueryParams.TeamRule = ESweepTeamRule::Enemies;
    TArray<AActor*> InIgnoreActors;
    InIgnoreActors.Add(PerformingActor);
    InIgnoreActors.Add(LineTraceResult.GetActor()); // If we found a LineTraceResult, we don't need to look for the closest Actor in the Sweep as that's the main target.
    AgoraSweepMultiBox(PerformingActor, PerformingActor, SweepQueryParams, HitBox / 2, SweepStartLocation, LookDirection, OutOverlappingActors, InIgnoreActors);


    if (bDebug)
    {
        DrawDebugBox(PerformingActor->GetWorld(), SweepStartLocation, (HitBox / 2), LookDirection.Quaternion(), FColor::Orange, true, -1.f, 0, 1.f);
    }

    FMeleeHitResult OutHandle;
    int32 PrimaryHitResultIndex = 0;
    if (LineTraceResult.Actor != nullptr && !UAgoraTeamLibrary::IsFriendly(LineTraceResult.GetActor(), PerformingActor))
    {
        OutHandle.PrimaryTarget = LineTraceResult.GetActor();
    }
    else // We didn't find a result with the line trace, so now we try and find the closest enemy in the sweep
    {
        float ClosestTargetDistance = TNumericLimits<float>::Max(); // High number to make sure it always gets triggered, we crash otherwise

        for (int32 Index = 0; Index < OutOverlappingActors.Num(); Index++)
        {
            float NewTargetDistance = PerformingActor->GetSquaredDistanceTo(OutOverlappingActors[Index]);

            if (NewTargetDistance <= ClosestTargetDistance)
            {
                ClosestTargetDistance = NewTargetDistance;
                OutHandle.PrimaryTarget = OutOverlappingActors[Index];
                PrimaryHitResultIndex = Index;
            }
        }
        if (OutOverlappingActors.Num() > 0)
        {
            OutOverlappingActors.RemoveAt(PrimaryHitResultIndex); //the primary hit target is inside OutHandle.PrimaryTarget, so we don't need it to be inside OutOverlappingActors
        }
    }

    //why the tset fuckery?
    //because the fucking bullshit overlap up top refuses to acknowledge InIgnoredActors.
    //So fuck it.
    //fuck.
    TSet<AActor*> OverlappingTargets;
    for (AActor* Target : OutOverlappingActors)
    {
        OverlappingTargets.Emplace(Target);
    }
    OverlappingTargets.Remove(LineTraceResult.GetActor());
    OverlappingTargets.Remove(OutHandle.PrimaryTarget);
    OutHandle.OverlappingTargets = OverlappingTargets.Array();
    return OutHandle;
}

bool UAgoraSweepLibrary::AgoraSweepMultiSphere(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, float SweepSphereRadius, const FVector& SweepLocation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors)
{
    if (!WorldContextObject || !WorldContextObject->ImplementsGetWorld()) { return false; }

    FCollisionQueryParams CollisionQueryParams;
    CollisionQueryParams.bTraceComplex = false;
    CollisionQueryParams.bReturnPhysicalMaterial = false;
    CollisionQueryParams.AddIgnoredActors(IgnoredActors);

    return UAgoraSweepLibrary::AgoraSweepMulti(WorldContextObject, SweepingActor, 
                                               AgoraQueryParams, OutOverlappingActors, 
                                               SweepLocation, FQuat::Identity,
                                               FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(SweepSphereRadius), 
                                               CollisionQueryParams);
}

bool UAgoraSweepLibrary::AgoraSweepMultiBox(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, const FVector& SweepBoxExtent, const FVector& SweepLocation, const FRotator& SweepRotation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors)
{
    if (!WorldContextObject || !WorldContextObject->ImplementsGetWorld()) { return false; }

    FCollisionQueryParams CollisionQueryParams;
    CollisionQueryParams.bTraceComplex = false;
    CollisionQueryParams.bReturnPhysicalMaterial = false;
    CollisionQueryParams.AddIgnoredActors(IgnoredActors);

    return UAgoraSweepLibrary::AgoraSweepMulti(WorldContextObject, SweepingActor, 
                                               AgoraQueryParams, OutOverlappingActors, 
                                               SweepLocation, SweepRotation.Quaternion(),
                                               FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeBox(SweepBoxExtent), 
                                               CollisionQueryParams);
}

bool UAgoraSweepLibrary::AgoraSweepMultiCapsule(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, const float SweepCapsuleRadius, const float SweepCapsuleHalfHeight, const FVector& SweepLocation, const FRotator& SweepRotation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors)
{
    if (!WorldContextObject || !WorldContextObject->ImplementsGetWorld()) { return false; }

    FCollisionQueryParams CollisionQueryParams;
    CollisionQueryParams.bTraceComplex = false;
    CollisionQueryParams.bReturnPhysicalMaterial = false;
    CollisionQueryParams.AddIgnoredActors(IgnoredActors);

    return UAgoraSweepLibrary::AgoraSweepMulti(WorldContextObject, SweepingActor, 
                                               AgoraQueryParams, OutOverlappingActors, 
                                               SweepLocation, SweepRotation.Quaternion(),
                                               FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeCapsule(SweepCapsuleRadius, SweepCapsuleHalfHeight), 
                                               CollisionQueryParams);
}

bool UAgoraSweepLibrary::AgoraSweepMultiPyramid(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, const float BaseWidth, const float BaseHeight, const FVector& PyramidOrigin, const float PyramidHeight, const FRotator& PyramidRotation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors, const FVector2D& Spread /*= FVector2D(1.0f, 1.0f)*/, bool bDebug /*= false*/)
{
    if (!WorldContextObject || !WorldContextObject->ImplementsGetWorld()) { return false; }

    if (PyramidHeight < 1 || BaseWidth <= 0.0f || BaseHeight <= 0.0f || PyramidOrigin == FVector::ZeroVector) { return false; }

    FVector2D ClampedSpread(FMath::Clamp(Spread.X, 0.0f, 1.0f), FMath::Clamp(Spread.Y, 0.0f, 1.0f));

    FCollisionQueryParams CollisionQueryParams;
    CollisionQueryParams.bTraceComplex = false;
    CollisionQueryParams.bReturnPhysicalMaterial = false;
    CollisionQueryParams.AddIgnoredActors(IgnoredActors);

    int NumberOfPoints = BaseWidth / 2;

    //define points on base with respect to the x axis
    TSet<FVector> PointsOnBase;
    PointsOnBase.Reserve(NumberOfPoints);
    int Idx = 0;
    while (Idx++ < NumberOfPoints)
    {

        float Y = (ClampedSpread.X * (BaseWidth / 2)) * UKismetMathLibrary::MapRangeUnclamped(UAgoraUtilityLibrary::StdGaussian(), -2.5f, 2.5f, -1.0f, 1.0f); //I don't want the full range of the std normal
        float Z = (ClampedSpread.Y * (BaseHeight / 2)) * UKismetMathLibrary::MapRangeUnclamped(UAgoraUtilityLibrary::StdGaussian(), -2.5f, 2.5f, -1.0f, 1.0f);
        FVector Point(PyramidHeight, Y, Z);

        //rotate these points back to their desired direction, and then move these points so they're relative to the pyramid's origin
        FVector RotatedVector = UKismetMathLibrary::GreaterGreater_VectorRotator(Point, PyramidRotation);
        Point = RotatedVector + PyramidOrigin;
        PointsOnBase.Emplace(Point);
    }

    //run the trace on each vector
    TArray<FHitResult> HitResults;
    HitResults.Reserve(20);
    for (const FVector& EndVector : PointsOnBase)
    {
        HitResults.Empty();
        bool Success = WorldContextObject->GetWorld()->LineTraceMultiByObjectType(HitResults, PyramidOrigin, EndVector, FCollisionObjectQueryParams(ECC_Pawn), CollisionQueryParams);
        if (Success)
        {
            for (const FHitResult& Result : HitResults)
            {
                if (AgoraQueryParams.MatchesQuery(SweepingActor, Result.GetActor()))
                {
                    OutOverlappingActors.AddUnique(Result.GetActor());
                }
            }
        }
        if (bDebug) DrawDebugLine(WorldContextObject->GetWorld(), PyramidOrigin, EndVector, FColor::Green, true);
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
// Structs
//////////////////////////////////////////////////////////////////////////

FAgoraSweepQueryParams UAgoraSweepLibrary::MakeSweepParamsForEnemies(FAgoraSweepTagContainer RequiredTags, FAgoraSweepTagContainer RestrictedTags)
{
    FAgoraSweepQueryParams OutSweepParams;
    OutSweepParams.RequiredTags = RequiredTags.Tags;
    OutSweepParams.RequiredTagsRule = RequiredTags.TagQueryRule;

    OutSweepParams.RestrictedTags = RestrictedTags.Tags;
    OutSweepParams.RestrictedTagsRule = RestrictedTags.TagQueryRule;

    OutSweepParams.TeamRule = ESweepTeamRule::Enemies;

    return OutSweepParams;
}

FAgoraSweepQueryParams UAgoraSweepLibrary::MakeSweepParamsForAllies(FAgoraSweepTagContainer RequiredTags, FAgoraSweepTagContainer RestrictedTags)
{
    FAgoraSweepQueryParams OutSweepParams;
    OutSweepParams.RequiredTags = RequiredTags.Tags;
    OutSweepParams.RequiredTagsRule = RequiredTags.TagQueryRule;

    OutSweepParams.RestrictedTags = RestrictedTags.Tags;
    OutSweepParams.RestrictedTagsRule = RestrictedTags.TagQueryRule;

    OutSweepParams.TeamRule = ESweepTeamRule::Allies;
    
    return OutSweepParams;
}

bool FAgoraSweepQueryParams::PassesTeamCheck(const AActor* SweepingActor, const AActor* SweptActor) const
{
    // If we don't care about team affiliation, then we pass
    if (TeamRule == ESweepTeamRule::NoRule) { return true; }

    bool IsFriendly = UAgoraTeamLibrary::IsFriendly(SweepingActor, SweptActor);
    if (IsFriendly && TeamRule == ESweepTeamRule::Allies)
    {
        return true;
    }
    else if (!IsFriendly && TeamRule == ESweepTeamRule::Enemies)
    {
        return true;
    }

    // we're friendly and the team rule is enemies, or we're an enemy and the team rule is friendly
    return false;
}

bool FAgoraSweepQueryParams::PassesTagCheck(const AActor* SweptActor) const
{
    if (RequiredTags.IsEmpty() && RestrictedTags.IsEmpty()) { return true; }

    const IGameplayTagAssetInterface* SweptActorAsTagInterface = Cast<IGameplayTagAssetInterface>(SweptActor);
    if (!SweptActorAsTagInterface)
    {
        TRACESTATIC(Agora, Warning, "%s does not implement IGameplayTagAssetInterface", *GetNameSafe(SweptActor))
            // Technically in this case the actor doesn't have any tags, thus it doesn't have any restricted 
            if (RequiredTags.IsEmpty())
            {
                return true;
            }
        return false;
    }

    bool bHasRequiredTags = false;
    bool bHasRestrictedTags = false;

    // Required Tag Checks
    {
        if (RequiredTags.IsEmpty())
        {
            bHasRequiredTags = true;
        }
        else
        {
            bHasRequiredTags = RequiredTagsRule ==
                ESweepTagRule::All ? SweptActorAsTagInterface->HasAllMatchingGameplayTags(RequiredTags) :
                SweptActorAsTagInterface->HasAnyMatchingGameplayTags(RequiredTags);
        }
    }

    // Restricted Tag Checks
    {
        if (RestrictedTags.IsEmpty())
        {
            bHasRestrictedTags = false;
        }
        else
        {
            bHasRestrictedTags = RestrictedTagsRule ==
                ESweepTagRule::All ? SweptActorAsTagInterface->HasAllMatchingGameplayTags(RestrictedTags) :
                SweptActorAsTagInterface->HasAnyMatchingGameplayTags(RestrictedTags);
        }
    }

    return bHasRequiredTags && !bHasRestrictedTags;
}