// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Tasks/AbilityTask.h"
#include "GameplayEffectTypes.h"
#include "TimerManager.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraAbilityTask_WaitTicks.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitApplyEffectAreaDelegate, int32, TickNum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitTicksFinished);

class UGameplayAbility;
/**
 * Convenience task for doing something ticked
 */
UCLASS()
class AGORA_API UAgoraAbilityTask_WaitTicks : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FWaitApplyEffectAreaDelegate OnTick;

	UPROPERTY(BlueprintAssignable)
	FWaitTicksFinished OnFinished;

	virtual void Activate() override;
	void OnDestroy(bool AbilityEnded) override;

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Agora|Tasks")
		static UAgoraAbilityTask_WaitTicks* WaitTicks(
		UGameplayAbility* OwningAbility,
		float Duration = 0.0f, 
		int32 NumTicks = 1);

protected:
	UFUNCTION()
	void HandleTick(int32 TickNum);

	UPROPERTY(BlueprintReadOnly, Category = "WaitApplyEffectInArea")
	float Duration = 0;

	UPROPERTY(BlueprintReadOnly, Category = "WaitApplyEffectInArea")
	int32 NumTicks = 0;

	TArray<FTimerDelegate> TimerDelegates;
	TArray<FTimerHandle> Timers;

	FTimerManager& GetTimerManager();
};
