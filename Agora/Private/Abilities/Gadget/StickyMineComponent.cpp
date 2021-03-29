// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "StickyMineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AgoraEntityLibrary.h"
#include "AgoraTeamLibrary.h"
#include "GameFramework/Pawn.h"

UStickyMineComponent::UStickyMineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	//stuff this into a curvefloat table or something soon
	//it was generated in excel
	//e^(-.5x) quantized to 0.5 step (except the first value)
	BeepTimes.Emplace(0.5);
	BeepTimes.Emplace(0.60653066);
	BeepTimes.Emplace(0.472366553);
	BeepTimes.Emplace(0.367879441);
	BeepTimes.Emplace(0.286504797);
	BeepTimes.Emplace(0.22313016);
	BeepTimes.Emplace(0.173773943);
	BeepTimes.Emplace(0.135335283);
	BeepTimes.Emplace(0.105399225);
	BeepTimes.Emplace(0.082084999);
	BeepTimes.Emplace(0.063927861);
	BeepTimes.Emplace(0.049787068);
	BeepTimes.Emplace(0.038774208);
	BeepTimes.Emplace(0.030197383);
	BeepTimes.Emplace(0.023517746);
	BeepTimes.Emplace(0.018315639);
	BeepTimes.Emplace(0.014264234);
	BeepTimes.Emplace(0.011108997);
	BeepTimes.Emplace(0.008651695);
	BeepTimes.Emplace(0.006737947);

}

// Called when the game starts
void UStickyMineComponent::BeginPlay()
{
	Super::BeginPlay();
}

AActor* UStickyMineComponent::GetClosestActor(const TArray<AActor*>& Actors)
{
	AActor* ClosestActor = nullptr;
	for (AActor* Actor : Actors)
	{
		if (ClosestActor && !ClosestActor->IsPendingKill())
		{
			if (FVector::DistSquared(GetOwner()->GetActorLocation(), ClosestActor->GetActorLocation()) >
				FVector::DistSquared(Actor->GetActorLocation(), GetOwner()->GetActorLocation()))
			{
				ClosestActor = Actor;
			}
		}
		else
		{
			ClosestActor = Actor;
		}
	}
	return ClosestActor;
}

void UStickyMineComponent::ScheduleRecurringBeepTimer()
{
	if (IsBeeping)
	{
		if (BeepTimesIndex < BeepTimes.Num())
		{
			UGameplayStatics::PlaySoundAtLocation(GetOwner(), BeepSound, GetOwner()->GetActorLocation());
			GetOwner()->GetWorldTimerManager().SetTimer(BeepTimer, this, &UStickyMineComponent::ScheduleRecurringBeepTimer, BeepTimes[BeepTimesIndex++], false);
		}
		else 
		{
			StopBeeping();
		}
	}
}

AActor* UStickyMineComponent::GetBestTarget(const TArray<AActor*>& Actors)
{
	TArray<AActor*> FriendlyHeroes, EnemyHeroes, FriendlyMinions, EnemyMinions, Others;
	for (AActor* Actor : Actors)
	{
		switch (UAgoraEntityLibrary::GetEntityType(Actor))
		{
		case EDescriptor::Hero:
		{
		if (UAgoraTeamLibrary::IsFriendly(Actor, GetOwner()->GetInstigator()))
				FriendlyHeroes.Emplace(Actor);
			else
				EnemyHeroes.Emplace(Actor);
			break;
		}
		case EDescriptor::Minion:
		{
			if (UAgoraTeamLibrary::IsFriendly(Actor, GetOwner()->GetInstigator()))
				FriendlyMinions.Emplace(Actor);
			else
				EnemyMinions.Emplace(Actor);
			break;
		}
		case EDescriptor::JungleCreep:
		{
			Others.Emplace(Actor);
			break;
		}
		}
	}
	TArray<TArray<AActor*>> AllFoundActors;

	//this ordering is important: targets are prioritized in this order.
	AllFoundActors.Emplace(EnemyHeroes);
	AllFoundActors.Emplace(EnemyMinions);
	AllFoundActors.Emplace(FriendlyHeroes);
	AllFoundActors.Emplace(FriendlyMinions);
	AllFoundActors.Emplace(Others);
	for (const TArray<AActor*>& FoundActors : AllFoundActors)
	{
		if (FoundActors.Num() > 0)
		{
			return GetClosestActor(FoundActors);
		}
	}
	return nullptr;
}

void UStickyMineComponent::StartBeeping()
{
	IsBeeping = true;
	BeepTimesIndex = 0;
	GetOwner()->GetWorldTimerManager().SetTimer(BeepTimer, this, &UStickyMineComponent::ScheduleRecurringBeepTimer, BeepTimes[BeepTimesIndex++], false);
}

void UStickyMineComponent::StopBeeping()
{
	GetOwner()->GetWorldTimerManager().ClearTimer(BeepTimer);
	IsBeeping = false;
}

