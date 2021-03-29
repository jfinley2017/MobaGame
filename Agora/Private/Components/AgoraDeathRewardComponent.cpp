// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraDeathRewardComponent.h"
#include <AbilitySystemInterface.h>
#include <AbilitySystemComponent.h>
#include <GameplayEffect.h>
#include "DeathRewardTargetInterface.h"
#include "DeathRewardEffectStrategy.h"
#include "DeathRewardEffects/DeathEffectStrategy_SplitExperience.h"
#include "DeathRewardEffects/DeathEffectStrategy_GoldReward.h"
#include "DeathRewardTargeting_Killer.h"
#include "DeathRewardTargeting_Proximity.h"

UAgoraDeathRewardComponent::UAgoraDeathRewardComponent()
{
	// Provide defaults - most things will have these as their normal rewards
	FDeathRewardTargetPair XpReward = FDeathRewardTargetPair(UDeathEffectStrategy_SplitExperience::StaticClass(), UDeathRewardTargeting_Proximity::StaticClass());
	FDeathRewardTargetPair GoldReward = FDeathRewardTargetPair(UDeathEffectStrategy_GoldReward::StaticClass(), UDeathRewardTargeting_Killer::StaticClass());

	AddDeathRewardTargetPair(XpReward);
	AddDeathRewardTargetPair(GoldReward);
}

void UAgoraDeathRewardComponent::NotifyDied(FDamageContext KillingBlow)
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	FDamageContext Context = FDamageContext(KillingBlow);

	for (FDeathRewardTargetPair Pair : DeathRewardTargetPairs)
	{
		TArray<AActor*> RewardTargets;

		if (!ensure(Pair.IsValid()))
		{
			break;
		}

		Pair.GetEffectTargets(RewardTargets, Context);

		UGameplayEffect* Effect = Pair.GetRewardEffect(RewardTargets, Context);

		if (!ensure(Effect))
		{
			break;
		}

		for (const AActor* Target : RewardTargets)
		{

			const IAbilitySystemInterface* DeadActor = Cast<IAbilitySystemInterface>(KillingBlow.DamagedActor);
			const IAbilitySystemInterface* RewardedActor = Cast<IAbilitySystemInterface>(Target);
			if (!RewardedActor || !DeadActor)
			{
				break;
			}

			UAbilitySystemComponent* RewardedASC = RewardedActor->GetAbilitySystemComponent();
			UAbilitySystemComponent* DeadASC = DeadActor->GetAbilitySystemComponent();

			if (!RewardedASC || !DeadASC)
			{
				break;
			}

			DeadASC->ApplyGameplayEffectToTarget(Effect, RewardedASC, 1, RewardedASC->MakeEffectContext());
		}
	}
}

void UAgoraDeathRewardComponent::AddDeathRewardTargetPair(FDeathRewardTargetPair Pair)
{
	DeathRewardTargetPairs.Add(Pair);
}

UGameplayEffect* FDeathRewardTargetPair::GetRewardEffect(const TArray<AActor*>& InTargets, FDamageContext Context)
{
	if (!Reward.bUseEffectClass)
	{
		UObject* EffectStrategyCDO = Reward.GetRewardEffectStrategy->GetDefaultObject();
		IDeathRewardEffectStrategy* EffectStrategy = Cast<IDeathRewardEffectStrategy>(EffectStrategyCDO);

		return EffectStrategy->GetRewardEffect(InTargets, Context);
	}
	else if(Reward.GameplayEffect)
	{
		return Reward.GameplayEffect->GetDefaultObject<UGameplayEffect>();
	}

	return nullptr;
}

void FDeathRewardTargetPair::GetEffectTargets(TArray<AActor*>& OutTargets, FDamageContext Context)
{
	UObject* TargetStrategyCDO = GetRewardTargetsStrategy->GetDefaultObject();
	IDeathRewardTargetInterface* TargetInterface = Cast<IDeathRewardTargetInterface>(TargetStrategyCDO);

	TargetInterface->GetRewardTargets(OutTargets, Context);
}
