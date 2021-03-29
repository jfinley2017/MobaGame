// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include "Engine/LatentActionManager.h"
#include "WaitFocused.generated.h"

class IFocusableInterface;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitFocusedOutputPinSignature, AActor*, FocusingActor);

/**
 * 
 */
UCLASS()
class AGORA_API UWaitFocused : public UAgoraBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FWaitFocusedOutputPinSignature OnFocused;
    UPROPERTY(BlueprintAssignable)
    FWaitFocusedOutputPinSignature OnUnFocused;

    UWaitFocused(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitFocused* WaitFocused(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor);
    
    // UAgoraBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UAgoraBlueprintAsyncActionbase interface

    UFUNCTION()
    void NotifyFocused(AActor* FocusingActor);
    UFUNCTION()
    void NotifyUnFocused(AActor* FocusingActor);

private:

    FScriptDelegate OnFocusedDelegate;
    FScriptDelegate OnUnFocusedDelegate;
    
    TWeakObjectPtr<AActor> ObservedActor;
    IFocusableInterface* ObservedActorAsFocusableActor = nullptr;

};
