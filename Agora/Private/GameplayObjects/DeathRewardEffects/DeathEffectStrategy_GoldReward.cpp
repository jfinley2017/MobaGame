// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "DeathEffectStrategy_GoldReward.h"
#include <GameplayEffect.h>
#include "AgoraAttributeSetEconomyBase.h"
#include "AgoraDeathRewardComponent.h"
#include <AbilitySystemBlueprintLibrary.h>

UGameplayEffect* UDeathEffectStrategy_GoldReward::GetRewardEffect(const TArray<AActor*>& InActors, FDamageContext Context)
{
	FGameplayModifierInfo GoldGainModifier;
	GoldGainModifier.Attribute = UAgoraAttributeSetEconomyBase::GetGoldAttribute();
	GoldGainModifier.ModifierOp = EGameplayModOp::Additive;

	bool bFoundAttributeValue;
	float AmountFloat = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Context.DamagedActor, UAgoraAttributeSetEconomyBase::GetGoldKillRewardAttribute(), bFoundAttributeValue);

	if (!bFoundAttributeValue)
	{
		return nullptr;
	}

	FScalableFloat RewardAmount = FScalableFloat(AmountFloat);
	GoldGainModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(RewardAmount);

	UGameplayEffect* KillerRewardEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GoldRewardEffect")));
	KillerRewardEffect->Modifiers.Add(GoldGainModifier);
	KillerRewardEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

	return KillerRewardEffect;
}