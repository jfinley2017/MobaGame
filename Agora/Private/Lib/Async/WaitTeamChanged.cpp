// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "WaitTeamChanged.h"
#include "GameFramework/Actor.h"
#include "TeamInterface.h"

UWaitTeamChanged::UWaitTeamChanged(const FObjectInitializer& ObjectInitialize)
{
    TeamChangedDelegate.BindUFunction(this, "NotifyTeamChanged");
}

UWaitTeamChanged* UWaitTeamChanged::WaitTeamChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor)
{
    UWaitTeamChanged* BlueprintNode = NewAgoraAsyncTask<UWaitTeamChanged>(WorldContextObject, InLatentActionInfo);

    BlueprintNode->ObservedActorAsTeamInterface = Cast<ITeamInterface>(ObservedActor);
    BlueprintNode->ObservedActor = ObservedActor;
    ensure(BlueprintNode->ObservedActor);
    ensure(BlueprintNode->ObservedActorAsTeamInterface);

    return BlueprintNode;
}

void UWaitTeamChanged::Activate()
{
    Super::Activate();

    if (ObservedActorAsTeamInterface)
    {
        ObservedActorAsTeamInterface->GetTeamChangedDelegate().AddUnique(TeamChangedDelegate);
    }
}

void UWaitTeamChanged::Cleanup()
{
    if (ObservedActorAsTeamInterface)
    {
        ObservedActorAsTeamInterface->GetTeamChangedDelegate().AddUnique(TeamChangedDelegate);
    }

    Super::Cleanup();
}

void UWaitTeamChanged::NotifyTeamChanged(AActor* ChangedActor)
{
    TeamChanged.Broadcast(ChangedActor);
}
