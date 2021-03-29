// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "GameplayAbilityTarget_Crosshair.h"
#include "AbilitySystemComponent.h"
#include "AgoraCharacterBase.h"
#include "Engine/Engine.h"
#include "Agora.h"

////////////////////////////////
// Targeting Information 
bool FGameplayAbilityTargeting_Crosshair::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    CrosshairHitLocation.NetSerialize(Ar, Map, bOutSuccess);
    FiringLocation.NetSerialize(Ar, Map, bOutSuccess);

    Ar << ChargeTime;
    Ar << UniqueID;

    bOutSuccess = true;
    return true;
}

void AGameplayAbilityTarget_Crosshair::ConfirmTargetingAndContinue()
{
    // Fill out our custom target data struct
    FGameplayAbilityTargeting_Crosshair* TargetData = new FGameplayAbilityTargeting_Crosshair();

    FVector OutCrosshairHitLocation;
    //OwningCharacter->GetAimIntention(OutCrosshairHitLocation);

    TargetData->CrosshairHitLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
    TargetData->CrosshairHitLocation.LiteralTransform = FTransform(OutCrosshairHitLocation);

    TargetData->FiringLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
    TargetData->FiringLocation.LiteralTransform = FiringLocation;

    // @todo charge calculation stuff here

    TargetData->UniqueID = OwningAbility->GetUniqueID();

    FGameplayAbilityTargetDataHandle Handle(TargetData);
    TargetDataReadyDelegate.Broadcast(Handle);

}
