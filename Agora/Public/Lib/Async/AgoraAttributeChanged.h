// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include "Delegates/IDelegateInstance.h"
#include "AgoraTypes.h" 
#include "AttributeSet.h"
#include "AgoraAttributeChanged.generated.h"

/**
 * Represents the amount that an attribute has been changed
 */
USTRUCT(BlueprintType)
struct FAgoraOnAttributeChangeDataHandle
{
    GENERATED_BODY()

    FAgoraOnAttributeChangeDataHandle() {}

    FAgoraOnAttributeChangeDataHandle(const FOnAttributeChangeData& Data) :
        NewValue(Data.NewValue),
        OldValue(Data.OldValue)
    {
    }

    UPROPERTY(BlueprintReadOnly, Category = "AttributeChangeDataHandle")
        float NewValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "AttributeChangeDataHandle")
        float OldValue = 0.0f;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAgoraOnAttributeChangeData, const FAgoraOnAttributeChangeDataHandle&, Data);

class IAbilitySystemInterface;
class UAbilitySystemComponent;

/**
 * Async node which encapsulates the idea of a OnAttributeChanged event. 
 * Allows us to utilize the ASC's OnAttributeChanged delegates, and removes the need for mirroring a delegate for each attribute on a unit.
 */
UCLASS()
class AGORA_API UWaitAttributeChanged : public UAgoraBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:

    UWaitAttributeChanged(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitAttributeChanged* WaitAttributeChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* InObservedActor, const FGameplayAttribute InObservedAttribute);
     
    // UAgoraBlueprintAsyncActionBase
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UAgoraBlueprintAsyncActionBase

    /** The delegate that represents the out pin for this node */
    UPROPERTY(BlueprintAssignable)
    FAgoraOnAttributeChangeData OnAttributeChanged;

protected:

    /** This is the ASC where the attribute is from */
    UAbilitySystemComponent* OwningAbilitySystemComponent;

    /** Actor which we are observing */
    AActor* ObservedActor;

    /** The attribute which we are observing */
    FGameplayAttribute ObservedAttribute;

    /** A handle for our delegate, allows us to destroy this node correctly by removing ourselves from the ASC's multicast delegate */
    FDelegateHandle AttributeChangedDelegate;

    /** The function that we're binding to the ASC's OnAttributeChanged delegate. This gives us an UNEXPOSED struct FOnAttributeChangeData */
    void AttributeChanged(const FOnAttributeChangeData& Data);

    /** A conversion function which allows us to expose certain data to BP, and broadcasts that data */
    void BroadcastAttributeChanged(const FAgoraOnAttributeChangeDataHandle& Data);

   
};
