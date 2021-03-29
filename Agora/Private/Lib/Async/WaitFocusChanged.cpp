// Fill out your copyright notice in the Description page of Project Settings.


#include "WaitFocusChanged.h"
#include "FocusableInterface.h"
#include "GameFramework/Actor.h"

UWaitFocusChanged::UWaitFocusChanged(const FObjectInitializer& ObjectInitializer)
{
    FocusChangedDelegate.BindUFunction(this, "NotifyFocusChanged");
}

UWaitFocusChanged* UWaitFocusChanged::WaitFocusChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor)
{
    UWaitFocusChanged* BlueprintNode = NewAgoraAsyncTask<UWaitFocusChanged>(WorldContextObject, InLatentActionInfo);
    BlueprintNode->ObservedActorAsFocusable = Cast<IFocusableInterface>(ObservedActor);
	// #TODO: Maybe check if we're not testing so we can check this
    //ensure(BlueprintNode->ObservedActorAsFocusable);
    return BlueprintNode;
}

void UWaitFocusChanged::Activate()
{
    Super::Activate();

    if (ObservedActorAsFocusable)
    {
        ObservedActorAsFocusable->GetFocusChangedDelegate().AddUnique(FocusChangedDelegate);
    }
}

void UWaitFocusChanged::Cleanup()
{

    if (ObservedActorAsFocusable)
    {
        ObservedActorAsFocusable->GetFocusChangedDelegate().Remove(FocusChangedDelegate);
    }

    Super::Cleanup();

}

void UWaitFocusChanged::NotifyFocusChanged(AActor* NewFocus)
{
    FocusChanged.Broadcast(NewFocus);
}
