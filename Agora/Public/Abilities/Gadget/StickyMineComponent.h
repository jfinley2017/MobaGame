// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "StickyMineComponent.generated.h"

class USoundBase;

/*
 * A component containing utility functions for Gadget's stickymine ability, as well as state vars for the beeping. Primary motivation is to move the heavy BP spaghetti into here and clean up the actor's blueprint.
 * If this pattern repeats itself for other similar abilities, I may refactor this into a more generic template of some kind.
 * -paca
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGORA_API UStickyMineComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStickyMineComponent();

protected:
	virtual void BeginPlay() override;

	AActor* GetClosestActor(const TArray<AActor*>& Actors);

	bool IsBeeping = false;
	FTimerHandle BeepTimer;
	TArray<float> BeepTimes;

	int32 BeepTimesIndex = 0;
	void ScheduleRecurringBeepTimer();

public:	
	UFUNCTION(BlueprintCallable, Category = "StickyMineComponent")
	AActor* GetBestTarget(const TArray<AActor*>& Actors);

	//These functions in particular (at least their core logic of a timer of increasing frequency) will probably have utility outside of this component. Will refactor when that happens.

	UFUNCTION(BlueprintCallable, Category = "StickyMineComponent")
	void StartBeeping();

	UFUNCTION(BlueprintCallable, Category = "StickyMineComponent")
	void StopBeeping();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StickyMineComponent")
	USoundBase* BeepSound;
};
