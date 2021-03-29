// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "TimerManager.h"
#include "CoreMinimal.h"
#include "Engine.h"
#include "Agora.h"
#include "Kismet/GameplayStatics.h"

/**
 * A timer that counts down, firing a lambda on each tick and when it's finished. See https://developer.android.com/reference/android/os/CountDownTimer for details.
 * The caller must retain a strong reference to the timer or things will crash.
 */

struct FAgoraCountDownTimer
{
	UWorld* WorldContextObject;
	FTimerHandle FinishTimer;
	FTimerHandle TickTimer;
	FTimerDelegate OnFinish;
	FTimerDelegate OnTick;
	bool bIsTickingDown = false;
	float CountdownTime = 0.0f;
	float TickInterval = 0.0f;
	
public:
	FAgoraCountDownTimer() {}
	FAgoraCountDownTimer(UWorld* WorldContextObject, float CountdownTime, float TickInterval = 1.f);
	
	template<typename Functor>
	void SetOnTickLambda(Functor&& Lambda);

	template<typename Functor>
	void SetOnFinishLambda(Functor&& Lambda);

	//Start the timer. Multiple calls do nothing. Calling SetOnTickLambda or SetOnFinishLambda while the timer is ticking will crash the game.
	void Start();

	//Stops the timer. Multiple calls do nothing. It can be restarted with another call to Start().
	void Cancel();

	float GetTimeRemaining() const;
};

template<typename Functor>
void FAgoraCountDownTimer::SetOnTickLambda(Functor&& Lambda)
{
	if (bIsTickingDown) TRACESTATIC(Agora, Fatal, "Tried to set OnTickLambda while the countdowntimer is running!");
	OnTick.BindLambda(Lambda);
}

template<typename Functor>
void FAgoraCountDownTimer::SetOnFinishLambda(Functor&& Lambda)
{
	if (bIsTickingDown) TRACESTATIC(Agora, Fatal, "Tried to set OnFinishLambda while the countdowntimer is running!");
	auto OnFinishLambda = [=] {
		UGameplayStatics::GetGameInstance(WorldContextObject)->GetTimerManager().ClearTimer(TickTimer);
		bIsTickingDown = false;
		Lambda();
	};
	OnFinish.BindLambda(OnFinishLambda);
}