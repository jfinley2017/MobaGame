// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "WaitRespawnBegin.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AgoraGameplayTags.h"
#include "AgoraEntityLibrary.h"

UWaitRespawnBegin::UWaitRespawnBegin()
{

}

UWaitRespawnBegin* UWaitRespawnBegin::WaitRespawnBegin(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor)
{
    UWaitRespawnBegin* OutNode = NewAgoraAsyncTask<UWaitRespawnBegin>(WorldContextObject, InLatentActionInfo);

    OutNode->ObservedActor = ObservedActor;
    IAbilitySystemInterface* ObservedActorAsAbilitySystemInterface = Cast<IAbilitySystemInterface>(ObservedActor);
    if (ObservedActorAsAbilitySystemInterface)
    {
        OutNode->ObservedAbilitySystemComponent = ObservedActorAsAbilitySystemInterface->GetAbilitySystemComponent();
    }

    return OutNode;

}

void UWaitRespawnBegin::Activate()
{
    Super::Activate();

    if (ObservedAbilitySystemComponent.Get())
    {
        ObservedAbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UWaitRespawnBegin::NotifyRespawnBegun);
        ObservedAbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &UWaitRespawnBegin::NotifyRespawnEnded);
    }

}

void UWaitRespawnBegin::Cleanup()
{

    if (ObservedAbilitySystemComponent.Get())
    {
        ObservedAbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);
        ObservedAbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);
    }

    Super::Cleanup();
}

void UWaitRespawnBegin::NotifyRespawnBegun(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{

    FGameplayTagContainer AssetTags;
    SpecApplied.GetAllAssetTags(AssetTags);

    FGameplayTagContainer GrantedTags;
    SpecApplied.GetAllGrantedTags(GrantedTags);

    if (AssetTags.HasTagExact(UAgoraGameplayTags::Dead()) || GrantedTags.HasTagExact(UAgoraGameplayTags::Dead()))
    {

        float RespawnTimestamp = 0.0f;
        if (UAgoraEntityLibrary::GetRespawnTimestamp(ObservedActor.Get(), RespawnTimestamp))
        {
            
            OnRespawnBegun.Broadcast(ObservedActor.Get(), RespawnTimestamp);
        }

        CachedActiveRespawnEffect = ActiveHandle;

    }
}

void UWaitRespawnBegin::NotifyRespawnEnded(const FActiveGameplayEffect& EffectRemoved)
{
    if (EffectRemoved.Handle == CachedActiveRespawnEffect)
    {
        CachedActiveRespawnEffect.Invalidate();
        OnRespawned.Broadcast();
    }
}

