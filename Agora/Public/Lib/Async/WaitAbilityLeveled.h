// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include "GameplayAbility.h"
#include "AgoraAbilitySystemComponent.h"
#include "WaitAbilityLeveled.generated.h"


class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitAbilityLeveledSignature, float, NewLevel);

/**
 * 
 */
UCLASS()
class AGORA_API UWaitAbilityLeveled : public UAgoraBlueprintAsyncActionBase
{
	GENERATED_BODY()
	

public:

    UPROPERTY(BlueprintAssignable)
    FWaitAbilityLeveledSignature OnAbilityLeveled;

    UWaitAbilityLeveled(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitAbilityLeveled* WaitAbilityLeveled(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* Observed, TSubclassOf<UGameplayAbility> Ability);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UBlueprintAsyncActionBase interface

    UFUNCTION()
    void NotifyAbilityLeveledup(TSubclassOf<UGameplayAbility> Ability, int32 NewLevel);

private:

    AActor* CachedObserved;
    UAgoraAbilitySystemComponent* CachedObservedAbilitySystem;
    TSubclassOf<UGameplayAbility> CachedAbility;

};
