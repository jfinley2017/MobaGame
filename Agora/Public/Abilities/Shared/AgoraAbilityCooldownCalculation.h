// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "AgoraAttributeSetStatsBase.h"
#include "AgoraAbilityCooldownCalculation.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraAbilityCooldownCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	UAgoraAbilityCooldownCalculation();

	UPROPERTY(EditDefaultsOnly, Category = "Intialization")
	FGameplayAttribute CapturedAttribute = UAgoraAttributeSetStatsBase::GetCooldownReductionAttribute();


	float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
	
protected:
	FGameplayEffectAttributeCaptureDefinition CaptureDefinition;
};
