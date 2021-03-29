// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraAbilityCooldownCalculation.h"
#include "AgoraGameplayAbility.h"
#include "AgoraAttributeSetStatsBase.h"
#include <GameplayEffectTypes.h>
#include "AgoraAbilityLibrary.h"
#include <GameplayTagContainer.h>

UAgoraAbilityCooldownCalculation::UAgoraAbilityCooldownCalculation()
{
	CaptureDefinition = FGameplayEffectAttributeCaptureDefinition(CapturedAttribute, EGameplayEffectAttributeCaptureSource::Source, true);

	RelevantAttributesToCapture.Add(CaptureDefinition);
}

float UAgoraAbilityCooldownCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FGameplayEffectContextHandle Context = Spec.GetContext();
	const UGameplayAbility* Ability = Context.GetAbility();

	if (Ability)
	{
		const UAgoraGameplayAbility* AgAbility = Cast<UAgoraGameplayAbility>(Ability);
		if (AgAbility)
		{
			float AbilityBaseCooldown = AgAbility->GetAbilityStat(AgAbility->GetCooldownRowName());

			float ComputedMagnitude = -1;
			GetCapturedAttributeMagnitude(CaptureDefinition, Spec, FAggregatorEvaluateParameters(), ComputedMagnitude);

			// CDR starts at 0 and is clamped between -0.3, 0.3. 0.3 = 30% CDR
			return (1 - ComputedMagnitude) * AbilityBaseCooldown;
		}
	}

	TRACE(AgoraAbility, Error, "UAgoraAbilityCooldownCalculation must be used with a UAgoraGameplayAbility");
	return -1;
}
