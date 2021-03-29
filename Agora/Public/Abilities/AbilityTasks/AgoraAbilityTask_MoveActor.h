// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TimerManager.h"
#include "AgoraAbilityTask_MoveActor.generated.h"

class UCurveFloat;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAgoraMoveActorFinished);

/**
 * Simple task that smoothly moves an actor from point A to point B The alpha curve determines the duration.
 */
UCLASS()
class AGORA_API UAgoraAbilityTask_MoveActor : public UAbilityTask
{
	GENERATED_BODY()


public:
	UPROPERTY(BlueprintAssignable)
	FAgoraMoveActorFinished Finished;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "AgoraMoveActor", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAgoraAbilityTask_MoveActor* MoveActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, AActor* TargetActor, UCurveFloat* AlphaCurve, FVector StartLocation, FVector EndLocation);

	virtual void Activate() override;

	virtual void OnDestroy(bool bAbilityEnded) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "AgoraMoveActor")
	AActor* TargetActor;

	//Goes from 0 to 1 over some length of time
	UPROPERTY(BlueprintReadOnly, Category = "AgoraMoveActor")
	UCurveFloat* AlphaCurve;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraMoveActor")
	FVector StartLocation;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraMoveActor")
	FVector EndLocation;

	UPROPERTY()
	FTimerHandle TickingTimer;

	UPROPERTY()
	FTimerHandle TaskTimer;
};
