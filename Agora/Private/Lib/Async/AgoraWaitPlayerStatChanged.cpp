// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraWaitPlayerStatChanged.h"
#include "Lib/AgoraStatTrackingLibrary.h"
#include "GameFramework/PlayerState.h"
#include "AgoraStatTracker.h"

UAgoraWaitPlayerStatChanged::UAgoraWaitPlayerStatChanged(const FObjectInitializer& ObjectInitializer)
{

}

UAgoraWaitPlayerStatChanged* UAgoraWaitPlayerStatChanged::WaitPlayerStatChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, APlayerState* Player, const FString& StatName)
{
    UAgoraWaitPlayerStatChanged* BlueprintNode = NewAgoraAsyncTask<UAgoraWaitPlayerStatChanged>(WorldContextObject, InLatentActionInfo);
    ensure(Player);
    BlueprintNode->CachedPlayerState = Player;
    BlueprintNode->CachedStatName = StatName;
    return BlueprintNode;
}

void UAgoraWaitPlayerStatChanged::Activate()
{
    Super::Activate();

    if (CachedPlayerState)
    {
        UAgoraStatTracker* StatTracker = CachedPlayerState->FindComponentByClass<UAgoraStatTracker>();
        if (StatTracker)
        {
            StatTracker->GetStatChangedDelegate(CachedStatName, true).AddDynamic(this, &UAgoraWaitPlayerStatChanged::NotifyStatChanged);
        }
    }

}

void UAgoraWaitPlayerStatChanged::Cleanup()
{
    if (CachedPlayerState)
    {
        UAgoraStatTracker* StatTracker = CachedPlayerState->FindComponentByClass<UAgoraStatTracker>();
        if (StatTracker)
        {
            StatTracker->GetStatChangedDelegate(CachedStatName, false).RemoveAll(this);
        }
    }

    Super::Cleanup();
}

void UAgoraWaitPlayerStatChanged::NotifyStatChanged(const FString& StatName, float NewValue)
{
    OnPlayerStatChanged.Broadcast(StatName, NewValue);
}
