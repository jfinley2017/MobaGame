// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include "WaitFocusChanged.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitFocusingSignature, AActor*, FocusedActor);

class IFocusableInterface;
class AActor;

/**
 * 
 */
UCLASS()
class AGORA_API UWaitFocusChanged : public UAgoraBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintAssignable)
    FWaitFocusingSignature FocusChanged;

    UWaitFocusChanged(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitFocusChanged* WaitFocusChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UBlueprintAsyncActionBase interface

    UFUNCTION()
    void NotifyFocusChanged(AActor* NewFocus);

private:

    IFocusableInterface* ObservedActorAsFocusable;

    FScriptDelegate FocusChangedDelegate;

};
