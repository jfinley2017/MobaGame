#include "AgoraLaneSpawner.h"
#include "AgoraLaneMinion.h"
#include "AgoraMinionSpawner.h"
#include "TimerManager.h"
#include "Agora.h"
#include "AgoraXpLevelComponent.h"
#include "AgoraTimeLevelComp.h"
#include "AgoraLevelComponent.h"

void AAgoraLaneSpawner::BeginPlay()
{
    Super::BeginPlay();
    RefreshCurrentSpawnLevel();
}

void AAgoraLaneSpawner::SpawnWave()
{
	WaveCount++;
	if (!bIsSpawningSuper)
	{
		MeleeMinionsToSpawn = CurrentSpawnLevel.NumMeleeMinions;
		RangedMinionsToSpawn = CurrentSpawnLevel.NumRangedMinions;

			// Only spawn siege minions every 3 waves
		SiegeMinionsToSpawn = (WaveCount % 3 == 0) ? CurrentSpawnLevel.NumSiegeMinions : 0;
	}
	else
	{
		SuperMinionsToSpawn = 5;
	}

	GetWorld()->GetTimerManager().SetTimer(MinionWaveSpawningTimerHandle, this, &AAgoraLaneSpawner::SpawnWaveMinion, TimeBetweenMinions, true, 0.f);
}

void AAgoraLaneSpawner::NotifyLeveledUp(AActor* LeveledActor, int32 NewLevel, float LastLevelTime)
{
    ReceiveLeveledUp(LeveledActor, NewLevel, LastLevelTime);

    RefreshCurrentSpawnLevel();
}

void AAgoraLaneSpawner::SpawnWaveMinion()
{
	TSubclassOf<AAgoraLaneMinion> ClassToSpawn;

	if (MeleeMinionsToSpawn > 0)
	{
		MeleeMinionsToSpawn--;
		ClassToSpawn = MeleeMinionClass;
	}
	else if (RangedMinionsToSpawn > 0)
	{
		RangedMinionsToSpawn--;
		ClassToSpawn = RangedMinionClass;
	}
	else if (SiegeMinionsToSpawn > 0)
	{
		SiegeMinionsToSpawn--;
		ClassToSpawn = SiegeMinionClass;
	}
	else if (SuperMinionsToSpawn > 0)
	{
		SuperMinionsToSpawn--;
		ClassToSpawn = SuperMinionClass;
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(MinionWaveSpawningTimerHandle);
		return;
	}

	ESpawnActorCollisionHandlingMethod SpawnParams = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ensure(ClassToSpawn))
	{
		AAgoraLaneMinion* Minion = GetWorld()->SpawnActorDeferred<AAgoraLaneMinion>(ClassToSpawn, GetActorTransform(), nullptr, nullptr, SpawnParams);
        Minion->SetTeam(Team);
		Minion->Waypoints = Waypoints;
		Minion->FinishSpawning(GetActorTransform());
	}
}

void AAgoraLaneSpawner::SetIsSpawningSuper(bool isSuper)
{
	if (isSuper == bIsSpawningSuper)
	{
		return;
	}

	bIsSpawningSuper = isSuper;
}

bool AAgoraLaneSpawner::GetIsSpawningSuper()
{
	return bIsSpawningSuper;
}

ELaneEnum AAgoraLaneSpawner::GetLane() const
{
    return Lane;
}

void AAgoraLaneSpawner::RefreshCurrentSpawnLevel()
{
    FLaneSpawnLevel* SpawnLevel = LevelComp->GetTotalMinionCountForNextLevel();
    if (SpawnLevel)
    {
        CurrentSpawnLevel = *SpawnLevel;
    }
}
