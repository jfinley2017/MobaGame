// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AgoraAbilityTask_WaitInputPress.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitInputPressDelegate);


/**
 * Literal copy-paste of the built-in WaitInputPress (paca's attempt to make something better)
 * But this actually works, straight up.
 * Not sure why.
 * I also took out the extraneous boolean, and this one also doesn't keep track of time elapsed.
 * Not important right now.
 */
UCLASS()
class AGORA_API UAgoraAbilityTask_WaitInputPress : public UAbilityTask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FWaitInputPressDelegate OnPress;

	UFUNCTION()
	void Pressed();

	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category = "AgoraAbility|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static UAgoraAbilityTask_WaitInputPress* WaitInputPress(UGameplayAbility* OwningAbility);

protected:
	FDelegateHandle DelegateHandle;
};
