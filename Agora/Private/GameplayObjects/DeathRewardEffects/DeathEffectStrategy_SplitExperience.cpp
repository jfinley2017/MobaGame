// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "DeathEffectStrategy_SplitExperience.h"
#include "AgoraDeathRewardComponent.h"
#include <GameplayEffect.h>
#include "AgoraAttributeSetEconomyBase.h"
#include <AbilitySystemBlueprintLibrary.h>

UGameplayEffect* UDeathEffectStrategy_SplitExperience::GetRewardEffect(const TArray<AActor*>& InActors, FDamageContext Context)
{
	FGameplayModifierInfo ExperienceModifier;
	ExperienceModifier.Attribute = UAgoraAttributeSetEconomyBase::GetExperienceAttribute();
	ExperienceModifier.ModifierOp = EGameplayModOp::Additive;
	
	FAttributeBasedFloat AttributeMagnitude = FAttributeBasedFloat();
	AttributeMagnitude.Coefficient = InActors.Num() > 0 ? (1.0f/InActors.Num()) : 1.f;
	
	bool bFoundAttributeValue;
	float AmountFloat = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Context.DamagedActor, UAgoraAttributeSetEconomyBase::GetExperienceKillRewardAttribute(), bFoundAttributeValue);

	if (!bFoundAttributeValue)
	{
		return nullptr;
	}

	FScalableFloat RewardAmount = FScalableFloat(AmountFloat);
	ExperienceModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(RewardAmount);

	FGameplayEffectAttributeCaptureDefinition Capture = FGameplayEffectAttributeCaptureDefinition(UAgoraAttributeSetEconomyBase::GetExperienceKillRewardAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	AttributeMagnitude.BackingAttribute = Capture;

	UGameplayEffect* KillerRewardEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("SplitExperienceEffect")));
	KillerRewardEffect->Modifiers.Add(ExperienceModifier);
	KillerRewardEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

	return KillerRewardEffect;
}
