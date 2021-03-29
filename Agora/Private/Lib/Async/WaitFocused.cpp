// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "WaitFocused.h"
#include "GameFramework/Actor.h"
#include "Interfaces/FocusableInterface.h"

UWaitFocused::UWaitFocused(const FObjectInitializer& ObjectInitializer)
{
    OnFocusedDelegate.BindUFunction(this, "NotifyFocused");
    OnUnFocusedDelegate.BindUFunction(this, "NotifyUnFocused");
}

UWaitFocused* UWaitFocused::WaitFocused(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor)
{
    UWaitFocused* BlueprintNode = NewAgoraAsyncTask<UWaitFocused>(WorldContextObject, InLatentActionInfo);
    BlueprintNode->ObservedActor = ObservedActor;
    BlueprintNode->ObservedActorAsFocusableActor = Cast<IFocusableInterface>(ObservedActor);
    ensure(BlueprintNode->ObservedActorAsFocusableActor);
    return BlueprintNode;
    
}

void UWaitFocused::Activate()
{
    Super::Activate();

    if (ObservedActorAsFocusableActor)
    {
        ObservedActorAsFocusableActor->GetStartFocusedDelegate().AddUnique(OnFocusedDelegate);
        ObservedActorAsFocusableActor->GetEndFocusedDelegate().AddUnique(OnUnFocusedDelegate);
    }
}

void UWaitFocused::Cleanup()
{
    if (ObservedActorAsFocusableActor)
    {
        ObservedActorAsFocusableActor->GetStartFocusedDelegate().Remove(OnFocusedDelegate);
        ObservedActorAsFocusableActor->GetEndFocusedDelegate().Remove(OnUnFocusedDelegate);
    }
}

void UWaitFocused::NotifyFocused(AActor* FocusingActor)
{
    OnFocused.Broadcast(FocusingActor);
}

void UWaitFocused::NotifyUnFocused(AActor* FocusingActor)
{
    OnUnFocused.Broadcast(FocusingActor);
}
