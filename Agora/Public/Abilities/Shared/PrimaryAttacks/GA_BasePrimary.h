// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AgoraGameplayAbility.h"
#include "GA_BasePrimary.generated.h"

class AAgoraCharacterBase;

/**
 * 
 */
UCLASS(Abstract)
class AGORA_API UGA_BasePrimary : public UAgoraGameplayAbility
{
	GENERATED_BODY()

public:

    void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual uint8 GetGameplayTaskDefaultPriority() const { return FGameplayTasks::DefaultPriority; }

protected:

    
    // Making this a blueprint implementable event. Why?
    // A.) Each basic attack type has its own logic for how it should apply the basic attack penalty.
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "BasePrimary")
    void ApplyPrimaryAttackMovespeedPenalty();


};
