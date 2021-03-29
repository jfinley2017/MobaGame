// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraStatTrackingLibrary.h"
#include "Agora.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY(AgoraStatTracker);

void UAgoraStatTrackingLibrary::ModifyPlayerStat(APlayerState* Player, const FString& StatName, EStatModType ModifierType, float Value, bool CreateIfNoExist)
{
    if (!Player) { return; }
    if (!Player->HasAuthority()) { return; }

    UAgoraStatTracker* StatTracker = Player->FindComponentByClass<UAgoraStatTracker>();
    if (StatTracker)
    {
        StatTracker->ModifyStat(StatName, ModifierType, Value, CreateIfNoExist);
    }

    return;
}

float UAgoraStatTrackingLibrary::GetPlayerStat(APlayerState* Player, const FString& StatName, bool CreateIfNoExist)
{
    if (!Player) 
    { 
        TRACESTATIC(AgoraStatTracker, Warning, "Player was null when retreiving stat %s", *StatName);
        return 0.0f; 
    }

    UAgoraStatTracker* StatTracker = Player->FindComponentByClass<UAgoraStatTracker>();
    if (StatTracker)
    {
        return StatTracker->GetStatValue(StatName,CreateIfNoExist);
    }

    return 0.0f;
}

void UAgoraStatTrackingLibrary::GetStatsForPlayer(APlayerState* Player, FString& OutString)
{
    if (!Player) 
    { 
        TRACESTATIC(AgoraStatTracker, Warning, "Player was null when retreiving stats");
        return; 
    }

    UAgoraStatTracker* StatTracker = Player->FindComponentByClass<UAgoraStatTracker>();
    if (StatTracker)
    {
        StatTracker->CreateStatString(OutString);
    }
}
