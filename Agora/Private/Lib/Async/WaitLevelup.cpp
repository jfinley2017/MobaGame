// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "WaitLevelup.h"
#include "Components/AgoraLevelComponent.h"


UWaitLevelup::UWaitLevelup(const FObjectInitializer& ObjectInitializer)
{
    OnLeveledupDelegate.BindUFunction(this, "NotifyLeveledup");
}

UWaitLevelup* UWaitLevelup::WaitLevelup(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* Observed)
{
    UWaitLevelup* BlueprintNode = NewAgoraAsyncTask<UWaitLevelup>(WorldContextObject, InLatentActionInfo);
    BlueprintNode->ObservedActor = Observed;
    ensure(BlueprintNode->ObservedActor);
    BlueprintNode->ObservedActorLevelComponent = Observed->FindComponentByClass<UAgoraLevelComponent>();
    ensure(BlueprintNode->ObservedActorLevelComponent);
    return BlueprintNode;
}

void UWaitLevelup::Activate()
{
    if (ObservedActorLevelComponent)
    {
        ObservedActorLevelComponent->OnLeveledUp.AddUnique(OnLeveledupDelegate);
    }

    Super::Activate();
}

void UWaitLevelup::Cleanup()
{
    if (ObservedActorLevelComponent)
    {
        ObservedActorLevelComponent->OnLeveledUp.Remove(OnLeveledupDelegate);
    }
}

void UWaitLevelup::NotifyLeveledup(uint8 NewLevel)
{
    OnLeveledUp.Broadcast(ObservedActor, ObservedActorLevelComponent);
}
