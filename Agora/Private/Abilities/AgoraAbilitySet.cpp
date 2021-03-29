// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "AgoraGameplayAbility.h"
#include "GameplayAbilitySpec.h"

void UAgoraAbilitySet::GiveAbilities(UAbilitySystemComponent* ASC) const
{
    for (const FAgoraAbilityStartupMapping& StartupAbilityMapping : Abilities)
    {
        if (StartupAbilityMapping.AbilityClass)
        {
            ASC->GiveAbility(FGameplayAbilitySpec(StartupAbilityMapping.AbilityClass, StartupAbilityMapping.StartingLevel, (uint8)StartupAbilityMapping.Input, nullptr));
        }
    }
}
