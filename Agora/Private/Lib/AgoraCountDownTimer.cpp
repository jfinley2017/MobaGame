// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraCountDownTimer.h"

FAgoraCountDownTimer::FAgoraCountDownTimer(UWorld* WorldContextObject, float CountdownTime, float TickInterval)
{
	check(WorldContextObject);
	check(CountdownTime > 0.0f);
	check(TickInterval > 0.0f);
	this->WorldContextObject = WorldContextObject;
	this->CountdownTime = CountdownTime;
	this->TickInterval = TickInterval;
}

void FAgoraCountDownTimer::Start()
{
	if (!bIsTickingDown)
	{
		bIsTickingDown = true;
		UGameplayStatics::GetGameInstance(WorldContextObject)->GetTimerManager().SetTimer(FinishTimer, OnFinish, CountdownTime, false);
		UGameplayStatics::GetGameInstance(WorldContextObject)->GetTimerManager().SetTimer(TickTimer, OnTick, TickInterval, true, 0.0f);
	}
	else
	{
		TRACESTATIC(Agora, Warning, "Someone tried to Start() a countdown timer that is already ticking down!");
	}
}

void FAgoraCountDownTimer::Cancel()
{
	if (bIsTickingDown)
	{
		bIsTickingDown = false;
		UGameplayStatics::GetGameInstance(WorldContextObject)->GetTimerManager().ClearTimer(FinishTimer);
		UGameplayStatics::GetGameInstance(WorldContextObject)->GetTimerManager().ClearTimer(TickTimer);
	}
	else
	{
		TRACESTATIC(Agora, Warning, "Someone tried to Cancel() a countdown timer that is already cancelled!");
	}
}

float FAgoraCountDownTimer::GetTimeRemaining() const
{
	return UGameplayStatics::GetGameInstance(WorldContextObject)->GetTimerManager().GetTimerRemaining(FinishTimer);
}

