// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "WaitAbilityCharges.h"
#include <GameplayAbilitySpec.h>
#include <AbilitySystemInterface.h>
#include "AgoraGameplayAbility.h"

UWaitAbilityCharges* UWaitAbilityCharges::WaitAbilityChargesChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, const AActor* Observed, FGameplayAbilitySpecHandle AbilityHandle)
{
	UWaitAbilityCharges* BlueprintNode = NewAgoraAsyncTask<UWaitAbilityCharges>(WorldContextObject, InLatentActionInfo);
	
	ensure(Observed);
	BlueprintNode->CachedObservedActor = Observed;
	
	const IAbilitySystemInterface* ObservedAsAbilitySystemInterface = Cast<IAbilitySystemInterface>(Observed);
	ensure(ObservedAsAbilitySystemInterface);
	
	BlueprintNode->CachedOwningAbilitySystemComponent = ObservedAsAbilitySystemInterface->GetAbilitySystemComponent();
	ensure(BlueprintNode->CachedOwningAbilitySystemComponent);

	BlueprintNode->CachedHandle = AbilityHandle;

	return BlueprintNode;
}

void UWaitAbilityCharges::NotifyChargesChanged(const FGameplayTag ChangedTag, int32 NewCount)
{
	OnChargeCountChanged.Broadcast(NewCount);
}

void UWaitAbilityCharges::Activate()
{
	Super::Activate();

	FGameplayAbilitySpec* SpecHandle = CachedOwningAbilitySystemComponent->FindAbilitySpecFromHandle(CachedHandle);
	if (!SpecHandle)
	{
		return;
	}

	UGameplayAbility* InstancedAbility = SpecHandle->GetPrimaryInstance();
	UAgoraGameplayAbility* AgAbility = Cast<UAgoraGameplayAbility>(InstancedAbility);
	
	if (InstancedAbility && AgAbility)
	{
		FGameplayTagCountContainer& TagContainer = AgAbility->GetTagCountContainer();
		
		TagContainer.RegisterGameplayTagEvent(UAgoraGameplayTags::Charges(), EGameplayTagEventType::AnyCountChange).AddUObject(this, &UWaitAbilityCharges::NotifyChargesChanged);
	}
}

void UWaitAbilityCharges::Cleanup()
{
    OnChargeCountChanged.RemoveAll(this);
}
