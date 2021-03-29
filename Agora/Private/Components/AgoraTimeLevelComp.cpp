#include "AgoraTimeLevelComp.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UAgoraTimeLevelComp::BeginPlay()
{
	Super::BeginPlay();

	TArray<FName> RowNames = LevelTable->GetRowNames();
	MaxLevel = RowNames.Num();

	for (FName RowName : RowNames)
	{
		FTimerHandle LevelUpTimer;
		FTimeLevelUpData* Row = LevelTable->FindRow<FTimeLevelUpData>(RowName, TEXT(""));

		float SecondsLeftToLevel = bLevelsFromGameTime
			? Row->SecondsToLevelUp - GetWorld()->GetTimeSeconds()
			: Row->SecondsToLevelUp;

		if (Row->SecondsToLevelUp <= 0)
		{
			// This row exists just to provide base stats for level 1 - no level increase required
		}
		else if (SecondsLeftToLevel && Row->SecondsToLevelUp <= 0)
		{
            // @todo: need to override LevelUp to accomodate the fact that when units are spawned they may need to be caught up on level
            // if we're catching minions up, we want the last level time to be the highest index we have met in the level table. 
            // (enough time has passed for me to be level 5, thus the last time I leveled is equal to the amount it took to hit level 5)
			// This could happen multiple times for actors that are spawned in late in game (minions)
			SetLevel(CurrentLevel + 1);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(LevelUpTimer, this, &UAgoraTimeLevelComp::HandleLevelTimer, SecondsLeftToLevel, false);
		}
	}
}

void UAgoraTimeLevelComp::HandleLevelTimer()
{
    
	SetLevel(CurrentLevel + 1);
}
