// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.


#include "AgoraAttributeChanged.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AgoraTypes.h"

UWaitAttributeChanged::UWaitAttributeChanged(const FObjectInitializer& ObjectInitializer)
{
}

UWaitAttributeChanged* UWaitAttributeChanged::WaitAttributeChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* InObservedActor, const FGameplayAttribute InObservedAttribute)
{
    UWaitAttributeChanged* OutNode = NewAgoraAsyncTask<UWaitAttributeChanged>(WorldContextObject, InLatentActionInfo);
    if (InObservedActor)
    {
        OutNode->ObservedActor = InObservedActor;
        IAbilitySystemInterface* InObservedAsAbilitySystemInterface = Cast<IAbilitySystemInterface>(InObservedActor);
		if (InObservedAsAbilitySystemInterface)
		{
			OutNode->OwningAbilitySystemComponent = InObservedAsAbilitySystemInterface->GetAbilitySystemComponent();
		}

        OutNode->ObservedAttribute = InObservedAttribute;
    }
  
    return OutNode;
}

void UWaitAttributeChanged::Activate()
{
    Super::Activate();

    if (OwningAbilitySystemComponent)
    {
        AttributeChangedDelegate = OwningAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ObservedAttribute).AddUObject(this, &UWaitAttributeChanged::AttributeChanged);
    }
}

void UWaitAttributeChanged::Cleanup()
{
    Super::Cleanup();

    if (OwningAbilitySystemComponent)
    {
        OwningAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ObservedAttribute).Remove(AttributeChangedDelegate);
    }
}

void UWaitAttributeChanged::AttributeChanged(const FOnAttributeChangeData& Data)
{
    BroadcastAttributeChanged(FAgoraOnAttributeChangeDataHandle(Data));
}

void UWaitAttributeChanged::BroadcastAttributeChanged(const FAgoraOnAttributeChangeDataHandle& Data)
{
    OnAttributeChanged.Broadcast(Data);
}

