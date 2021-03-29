// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"

#include "AttributeSets/AgoraAttributeSetHealth.h"

#include "AgoraDamageExecutionCalculation.generated.h"

class UGameplayAbility;

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:

    UAgoraDamageExecutionCalculation();
    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
    static EDamageType DetermineDamageType(const FGameplayEffectSpec& Spec);
};



