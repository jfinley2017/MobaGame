// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraAbilityTask_MoveActor.h"
#include "Curves/CurveFloat.h"

UAgoraAbilityTask_MoveActor* UAgoraAbilityTask_MoveActor::MoveActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, AActor* TargetActor, UCurveFloat* AlphaCurve, FVector StartLocation, FVector EndLocation)
{
	UAgoraAbilityTask_MoveActor* Task = NewAbilityTask<UAgoraAbilityTask_MoveActor>(OwningAbility, TaskInstanceName);
	Task->TargetActor = TargetActor;
	Task->AlphaCurve = AlphaCurve;
	Task->StartLocation = StartLocation;
	Task->EndLocation = EndLocation;
	return Task;
}

void UAgoraAbilityTask_MoveActor::Activate()
{
	FTimerDelegate TaskTimerDelegate;
	TaskTimerDelegate.BindLambda([&]
	{
		GetWorld()->GetTimerManager().ClearTimer(TickingTimer);
		Finished.Broadcast();
	});
	float MinTime, MaxTime;
	AlphaCurve->GetTimeRange(MinTime, MaxTime);
	GetWorld()->GetTimerManager().SetTimer(TaskTimer, TaskTimerDelegate, MaxTime, false);

	FTimerDelegate TickingTimerDelegate;
	TickingTimerDelegate.BindLambda([&]
	{
		float AlphaTime = GetWorld()->GetTimerManager().GetTimerElapsed(TaskTimer);
		FVector LerpedLoc = FMath::Lerp(StartLocation, EndLocation, AlphaCurve->GetFloatValue(AlphaTime));
		TargetActor->SetActorLocation(LerpedLoc, true);
	});
	GetWorld()->GetTimerManager().SetTimer(TickingTimer, TickingTimerDelegate, 1.0f / 120.0f, true);
}

void UAgoraAbilityTask_MoveActor::OnDestroy(bool bAbilityEnded)
{
	GetWorld()->GetTimerManager().ClearTimer(TickingTimer);
	GetWorld()->GetTimerManager().ClearTimer(TaskTimer);
	Super::OnDestroy(bAbilityEnded);
}
