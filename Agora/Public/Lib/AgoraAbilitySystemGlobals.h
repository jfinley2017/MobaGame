// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AgoraAbilitySystemGlobals.generated.h"

/**
 * See UAbilitySystemGlobals.
 * We're subclassing from here to override some methods, set in [/Script/GameplayAbilities.AbilitySystemGlobals] in DefaultGame.ini
 */
UCLASS(config = Game)
class AGORA_API UAgoraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	
public:
    /** Should allocate a project specific GameplayEffectContext struct. Caller is responsible for deallocation */
    virtual FGameplayEffectContext* AllocGameplayEffectContext() const;
};
