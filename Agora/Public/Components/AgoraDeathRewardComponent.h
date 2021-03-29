// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <GameplayEffect.h>
#include "AgoraTypes.h"
#include "AgoraDeathRewardComponent.generated.h"

class UAbilitySystemComponent;
class AActor;

USTRUCT(BlueprintType)
struct FDeathReward
{
	GENERATED_BODY()

public:
	FDeathReward() {};
	FDeathReward(TSubclassOf<UObject> GetRewardEffectStrategy): bUseEffectClass(false), GetRewardEffectStrategy(GetRewardEffectStrategy) {};
	FDeathReward(TSubclassOf<UGameplayEffect> GameplayEffect) : bUseEffectClass(true), GameplayEffect(GameplayEffect) {};

	virtual ~FDeathReward() {};

	bool IsValid() { return GameplayEffect || GetRewardEffectStrategy->IsValidLowLevelFast(); }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bUseEffectClass = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bUseEffectClass"))
	TSubclassOf<UGameplayEffect> GameplayEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (MustImplement = "DeathRewardEffectStrategy", EditCondition = "!bUseEffectClass"))
	TSubclassOf<UObject> GetRewardEffectStrategy;
};

/**
 * Associate a death reward to a group of targets
 * Implement the targeting or reward effect calculation strategy interfaces
 * To add more ways of associating effects to lists of actors
 */
USTRUCT(BlueprintType)
struct FDeathRewardTargetPair
{
	GENERATED_BODY()

public:
	FDeathRewardTargetPair() {};
	virtual ~FDeathRewardTargetPair() {};

	FDeathRewardTargetPair(FDeathReward Reward, TSubclassOf<UObject> GetRewardTargetsStrategy) 
		: Reward(Reward), GetRewardTargetsStrategy(GetRewardTargetsStrategy)
	{};

	// Convenience constructor, to avoid having to also make an FDeathReward to initialize
	FDeathRewardTargetPair(TSubclassOf<UObject> GetEffectStrategy, TSubclassOf<UObject> GetRewardTargetsStrategy)
		: Reward(FDeathReward(GetEffectStrategy)), GetRewardTargetsStrategy(GetRewardTargetsStrategy)
	{};

	/**
	 * Get the reward gameplay effect, handling the condition of the FDeathReward providing a class or an instance of a GE
	 */
	UGameplayEffect* GetRewardEffect(const TArray<AActor*>& InTargets, FDamageContext Context);
	
	/**
	 * Use the targeting strategy to get an array of actors to give our reward to
	 */
	void GetEffectTargets(TArray<AActor*>& OutTargets, FDamageContext Context);

	bool IsValid() { return Reward.IsValid() && GetRewardTargetsStrategy->IsValidLowLevelFast(); }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDeathReward Reward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (MustImplement = "DeathRewardTargetInterface"))
	TSubclassOf<UObject> GetRewardTargetsStrategy;
};

/**
 * Intended to handle death rewards which are defined by the 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGORA_API UAgoraDeathRewardComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAgoraDeathRewardComponent();

	void NotifyDied(FDamageContext KillingBlow);

	void AddDeathRewardTargetPair(FDeathRewardTargetPair Pair);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Agora)
	TArray<FDeathRewardTargetPair> DeathRewardTargetPairs;
};
