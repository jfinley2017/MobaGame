// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include <GameplayAbilitySpec.h>
#include <GameplayTagContainer.h>
#include "WaitAbilityCharges.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChargeCountChanged, int32, ChargeCount);

class UAbilitySystemComponent;
class AActor;

/**
 * Waits for the amount of ability charges to change on a given actor + ability handle
 */
UCLASS()
class AGORA_API UWaitAbilityCharges : public UAgoraBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintAssignable)
    FChargeCountChanged OnChargeCountChanged;

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
    virtual void Cleanup() override;
	// ~UBlueprintAsyncActionBase interface

	UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
	static UWaitAbilityCharges* WaitAbilityChargesChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, const AActor* Observed, FGameplayAbilitySpecHandle AbilityHandle);

	// This could be done, I'm not going to do it until we have an actor that actually has charges though
	//static UWaitAbilityCharges* WaitAbilityChargesChangedBySlot(const AActor* Observed, FGameplayAbilitySpecHandle AbilityHandle, EAbilityInput);

	UFUNCTION()
	void NotifyChargesChanged(const FGameplayTag ChangedTag, int32 NewCount);

protected:

    const AActor* CachedObservedActor;
    UAbilitySystemComponent* CachedOwningAbilitySystemComponent;
    FGameplayAbilitySpecHandle CachedHandle;

};
