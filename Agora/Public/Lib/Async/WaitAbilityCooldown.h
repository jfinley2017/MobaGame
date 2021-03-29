// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "WaitAbilityCooldown.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitAbillityCooldownChangedSignature);

class UGameplayAbility;
class UAbilitySystemComponent;

/**
 * Waits for an ability to become blocked, then reports which tag blocked it along with the duration
 */
UCLASS()
class AGORA_API UWaitAbilityCooldownChanged : public UAgoraBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:

    /** Called when the server adds the cooldown tag */
    UPROPERTY(BlueprintAssignable)
    FWaitAbillityCooldownChangedSignature AuthoritativeCooldownBegin;
    /** Called when the client adds the cooldown tag, this will trampled when the server sends its cooldown effect down */
    UPROPERTY(BlueprintAssignable)
    FWaitAbillityCooldownChangedSignature PredictedCooldownBegin;
    UPROPERTY(BlueprintAssignable)
    FWaitAbillityCooldownChangedSignature CooldownUpdated;
    UPROPERTY(BlueprintAssignable)
    FWaitAbillityCooldownChangedSignature CooldownEnd;

    UWaitAbilityCooldownChanged(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitAbilityCooldownChanged* WaitAbillityCooldownChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* Observed, TSubclassOf<UGameplayAbility> Ability);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UBlueprintAsyncActionBase interface

    UFUNCTION()
    void NotifyActiveGameplayEffectAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);
    UFUNCTION()
    void NotifyActiveGameplayEffectChanged(FActiveGameplayEffectHandle& ChangedHandle);
    UFUNCTION()
    void NotifyTagChanged(FGameplayTag ChangedTag, int32 NewCount);


private:

  
    AActor* CachedObservedActor;
    UAbilitySystemComponent* CachedOwningAbilitySystemComponent;
    TSubclassOf<UGameplayAbility> CachedGameplayAbility;
    const FGameplayTagContainer* CachedCooldownTags;
    
};
