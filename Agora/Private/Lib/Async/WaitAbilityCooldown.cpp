// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "WaitAbilityCooldown.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AgoraAbilityLibrary.h"


UWaitAbilityCooldownChanged::UWaitAbilityCooldownChanged(const FObjectInitializer& ObjectInitializer)
{

}

UWaitAbilityCooldownChanged* UWaitAbilityCooldownChanged::WaitAbillityCooldownChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* Observed, TSubclassOf<UGameplayAbility> GameplayAbility)
{
    UWaitAbilityCooldownChanged* BlueprintNode = NewAgoraAsyncTask<UWaitAbilityCooldownChanged>(WorldContextObject, InLatentActionInfo);
    ensure(Observed);
    BlueprintNode->CachedObservedActor = Observed;
    IAbilitySystemInterface* ObservedAsAbilitySystemInterface = Cast<IAbilitySystemInterface>(Observed);
    ensure(ObservedAsAbilitySystemInterface);
    BlueprintNode->CachedOwningAbilitySystemComponent = ObservedAsAbilitySystemInterface->GetAbilitySystemComponent();
    ensure(BlueprintNode->CachedOwningAbilitySystemComponent);

    BlueprintNode->CachedGameplayAbility = GameplayAbility;


    return BlueprintNode;
}

void UWaitAbilityCooldownChanged::Activate()
{
    Super::Activate();

    if (ensure(CachedOwningAbilitySystemComponent))
    {
        CachedOwningAbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UWaitAbilityCooldownChanged::NotifyActiveGameplayEffectAdded);

        if (ensure(CachedGameplayAbility))
        {
            CachedCooldownTags = UAgoraAbilityLibrary::GetCooldownTags(CachedGameplayAbility);
            if (CachedCooldownTags->IsValid())
            {
                for (FGameplayTag CooldownTag : *CachedCooldownTags)
                {
                    CachedOwningAbilitySystemComponent->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UWaitAbilityCooldownChanged::NotifyTagChanged);
                }
            }
        }
    }
}

void UWaitAbilityCooldownChanged::Cleanup()
{
    if (CachedOwningAbilitySystemComponent)
    {
        CachedOwningAbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);

        if (CachedCooldownTags)
        {
            for (FGameplayTag CooldownTag : *CachedCooldownTags)
            {
                CachedOwningAbilitySystemComponent->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
            }
        }
    }
}

void UWaitAbilityCooldownChanged::NotifyActiveGameplayEffectAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
    FGameplayTagContainer AssetTags;
    SpecApplied.GetAllAssetTags(AssetTags);
    
    FGameplayTagContainer GrantedTags;
    SpecApplied.GetAllGrantedTags(GrantedTags);

    if (CachedCooldownTags)
    {
        for (FGameplayTag CooldownTag : *CachedCooldownTags)
        {
            if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
            {
                ensure(CachedOwningAbilitySystemComponent);
                bool IsAuthority = CachedOwningAbilitySystemComponent->GetOwnerRole() == ROLE_Authority;

                // The server is running this node
                if (IsAuthority)
                {
                    AuthoritativeCooldownBegin.Broadcast();
                }

                // We are not the server but the SpecApplied is the one which was replicated from the server
                if (!IsAuthority && !SpecApplied.GetContext().GetAbilityInstance_NotReplicated())
                {
                    AuthoritativeCooldownBegin.Broadcast();
                }

                // The client is running this node and the SpecApplied is predicted (will be trampled by server)
                if (!IsAuthority && SpecApplied.GetContext().GetAbilityInstance_NotReplicated())
                {
                    PredictedCooldownBegin.Broadcast();
                }

            }
        }
    }
}

void UWaitAbilityCooldownChanged::NotifyActiveGameplayEffectChanged(FActiveGameplayEffectHandle& ChangedHandle)
{
    ensure(CachedOwningAbilitySystemComponent);

    // todo
}

void UWaitAbilityCooldownChanged::NotifyTagChanged(FGameplayTag ChangedTag, int32 NewCount)
{
    if (NewCount == 0)
    {
        CooldownEnd.Broadcast();
    }
}
