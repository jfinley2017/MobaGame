// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include "WaitTeamChanged.generated.h"

class ITeamInterface;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTeamChangedSignature, AActor*, Observed);

/**
 * 
 */
UCLASS()
class AGORA_API UWaitTeamChanged : public UAgoraBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FWaitTeamChangedSignature TeamChanged;

    UWaitTeamChanged(const FObjectInitializer& ObjectInitialize);

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitTeamChanged* WaitTeamChanged(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    //  ~UBlueprintAsyncActionBase interface

    UFUNCTION()
    void NotifyTeamChanged(AActor* ChangedActor);

private:
    

    AActor* ObservedActor;
    ITeamInterface* ObservedActorAsTeamInterface;

    FScriptDelegate TeamChangedDelegate;
};
