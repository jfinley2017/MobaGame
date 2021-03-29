// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "AgoraTypes.h"
#include "GameplayAbility.h"
#include "Agora.h"
#include "AgoraBlueprintFunctionLibrary.h" // Get context tags

FDamageContext::FDamageContext(const FGameplayEventData& GameplayEventData)
{
    DamagedActor = const_cast<AActor*>(GameplayEventData.Target);
    DamageInstigator = const_cast<AActor*>(GameplayEventData.Instigator);
    Magnitude = GameplayEventData.EventMagnitude;
    DamagingEffectContext = GameplayEventData.ContextHandle;

    DamageType = UAgoraBlueprintFunctionLibrary::DetermineDamageTypeFromContextHandle(GameplayEventData.ContextHandle);
    if (DamageType == EDamageType::DMGInvalid)
    {
        TRACESTATIC(Agora, Error, "Damage Context %s invalid damage type.", *ToString());
    }

}

FString FDamageContext::ToString()
{
    FString AbilityName = NULLSTRING;
    if (DamagingEffectContext.GetAbility())
    {
        DamagingEffectContext.GetAbility()->GetName(AbilityName);
    }
    
    FGameplayTagContainer ContextTagsContainer;
    UAgoraBlueprintFunctionLibrary::GetGameplayEffectContextTags(DamagingEffectContext, ContextTagsContainer);

    return FString::Printf(TEXT("%s->%s. Magnitude: %f, DamageType: %s, Ability: %s, Tags: %s"),
        *GetNameSafe(DamageInstigator), *GetNameSafe(DamagedActor), Magnitude, *(GETENUMSTRING("EDamageType", DamageType)),
         *AbilityName, *ContextTagsContainer.ToString());
}

bool FAgoraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    Super::NetSerialize(Ar, Map, bOutSuccess);

    ContextTags.NetSerialize(Ar, Map, bOutSuccess);

    bOutSuccess = true;
    return true;
}
