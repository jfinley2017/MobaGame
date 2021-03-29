// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include "WaitLevelup.generated.h"

class UAgoraLevelComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWaitLevelUpSignature, AActor*, LeveledActor, UAgoraLevelComponent*, LevelComponent);

/**
 * 
 */
UCLASS()
class AGORA_API UWaitLevelup : public UAgoraBlueprintAsyncActionBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintAssignable)
    FWaitLevelUpSignature OnLeveledUp;
    
    UWaitLevelup(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitLevelup* WaitLevelup(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* Observed);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UBlueprintAsyncActionBase interface

    UFUNCTION()
    void NotifyLeveledup(uint8 NewLevel);

private:

   

    FScriptDelegate OnLeveledupDelegate;

    AActor* ObservedActor;
    UAgoraLevelComponent* ObservedActorLevelComponent;

};
