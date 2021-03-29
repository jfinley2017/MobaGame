// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraAbilityTask_WaitTicks.h"
#include "AgoraAbilityLibrary.h"
#include "TimerManager.h"

void UAgoraAbilityTask_WaitTicks::Activate()
{
	float TickDuration = Duration <= 0 ? 0.0f : Duration / NumTicks;

	// if I have 5 ticks over 4 seconds, I want to guarantee that I get those 5 ticks
	for (int32 TickIndex = 0; TickIndex < NumTicks; TickIndex++)
	{
		if (TickIndex == 0)
		{
			HandleTick(TickIndex);
		}
		else
		{
			FTimerHandle TickTimer;
			FTimerDelegate TimerCallback;
			TimerDelegates.Add(TimerCallback);
			Timers.Add(TickTimer);
			TimerCallback.BindUObject(this, &UAgoraAbilityTask_WaitTicks::HandleTick, TickIndex);

			GetTimerManager().SetTimer(TickTimer, TimerCallback, TickIndex * TickDuration, false);
		}
	}
}

void UAgoraAbilityTask_WaitTicks::OnDestroy(bool AbilityEnded)
{
	for (auto Timer : Timers)
	{
		if (GetTimerManager().IsTimerActive(Timer))
		{
			TRACE(AgoraAbility, Warning, "WaitTicks was destroyed after %d seconds out of duration %d", GetTimerManager().GetTimerElapsed(Timer), Duration);
		}

		GetTimerManager().ClearTimer(Timer);
	}
	for (auto TD : TimerDelegates)
	{
		TD.Unbind();
	}

	Super::OnDestroy(AbilityEnded);
}

UAgoraAbilityTask_WaitTicks* UAgoraAbilityTask_WaitTicks::WaitTicks(UGameplayAbility* OwningAbility, float Duration /*= 0.0f*/, int32 NumTicks /*= 1*/)
{
	UAgoraAbilityTask_WaitTicks* Task = NewAbilityTask<UAgoraAbilityTask_WaitTicks>(OwningAbility);
	Task->Duration = Duration;
	Task->NumTicks = NumTicks;
	return Task;
}

void UAgoraAbilityTask_WaitTicks::HandleTick(int32 TickNum)
{
	if (!this)
	{
		return;
	}

	OnTick.Broadcast(TickNum);
	if (TickNum == (NumTicks - 1))
	{
		OnFinished.Broadcast();
	}
}

FTimerManager& UAgoraAbilityTask_WaitTicks::GetTimerManager()
{
	return GetGameplayTaskAvatar(this)->GetWorld()->GetTimerManager();
}
