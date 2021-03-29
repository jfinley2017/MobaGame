// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Lib/Async/AgoraBlueprintAsyncActionBase.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "WaitRespawnBegin.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWaitRespawnBeginSignature, AActor*, RespawningActor, float, RespawnFinishedTimestamp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitRespawnEndedSignature);
class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class AGORA_API UWaitRespawnBegin : public UAgoraBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintAssignable)
    FWaitRespawnBeginSignature OnRespawnBegun;

    UPROPERTY(BlueprintAssignable)
    FWaitRespawnBeginSignature OnRespawnTimerUpdated;

    UPROPERTY(BlueprintAssignable)
    FWaitRespawnEndedSignature OnRespawned;

    UWaitRespawnBegin();

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitRespawnBegin* WaitRespawnBegin(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UBlueprintAsyncActionBase interface

    UFUNCTION()
    void NotifyRespawnBegun(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);

    UFUNCTION()
    void NotifyRespawnEnded(const FActiveGameplayEffect& EffectRemoved);

private:

    FActiveGameplayEffectHandle CachedActiveRespawnEffect;
    TWeakObjectPtr<AActor> ObservedActor;
    TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
    FScriptDelegate RespawnBegunDelegate;

};
