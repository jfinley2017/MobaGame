// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include "AgoraWaitPlayerStatChanged.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWaitPlayerStatChangedSignature, const FString&, StatName, float, NewValue);

class APlayerState;

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraWaitPlayerStatChanged : public UAgoraBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintAssignable)
    FWaitPlayerStatChangedSignature OnPlayerStatChanged;

    UAgoraWaitPlayerStatChanged(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UAgoraWaitPlayerStatChanged* WaitPlayerStatChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, APlayerState* Player,  const FString& StatName);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UBlueprintAsyncActionBase interface

    UFUNCTION()
    void NotifyStatChanged(const FString& StatName, float NewValue);

protected:

   
    APlayerState* CachedPlayerState = nullptr;
    FString CachedStatName = "None";
	
};
