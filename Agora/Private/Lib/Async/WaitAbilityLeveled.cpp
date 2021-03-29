// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "WaitAbilityLeveled.h"

UWaitAbilityLeveled::UWaitAbilityLeveled(const FObjectInitializer& ObjectInitializer)
{

}

UWaitAbilityLeveled* UWaitAbilityLeveled::WaitAbilityLeveled(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* Observed, TSubclassOf<UGameplayAbility> Ability)
{
    UWaitAbilityLeveled* BlueprintNode = NewAgoraAsyncTask<UWaitAbilityLeveled>(WorldContextObject, InLatentActionInfo);
    ensure(Observed);
    BlueprintNode->CachedObserved = Observed;
    IAbilitySystemInterface* ObservedAsAbilitySystemInterface = Cast<IAbilitySystemInterface>(Observed);
    ensure(ObservedAsAbilitySystemInterface);
    BlueprintNode->CachedObservedAbilitySystem = Cast<UAgoraAbilitySystemComponent>(ObservedAsAbilitySystemInterface->GetAbilitySystemComponent());
    ensure(BlueprintNode->CachedObservedAbilitySystem);
    BlueprintNode->CachedAbility = Ability;

    return BlueprintNode;
}

void UWaitAbilityLeveled::Activate()
{
    Super::Activate();

    if (ensure(CachedObservedAbilitySystem))
    {
        CachedObservedAbilitySystem->OnAbilityLevelup.AddDynamic(this, &UWaitAbilityLeveled::NotifyAbilityLeveledup);
    }

}

void UWaitAbilityLeveled::Cleanup()
{
    if (CachedObservedAbilitySystem)
    {
        CachedObservedAbilitySystem->OnAbilityLevelup.RemoveAll(this);
    }

    Super::Cleanup();
}

void UWaitAbilityLeveled::NotifyAbilityLeveledup(TSubclassOf<UGameplayAbility> Ability, int32 NewLevel)
{
    if (Ability == CachedAbility)
    {
        OnAbilityLeveled.Broadcast(NewLevel);
    }
}
