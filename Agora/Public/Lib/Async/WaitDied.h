// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Lib/Async/AgoraBlueprintAsyncActionBase.h"
#include "AgoraTypes.h"
#include "WaitDied.generated.h"

class IDamageReceiverInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWaitDiedSignature, AActor*, KilledActor, const FDamageContext&, KillingBlow);

/**
 * 
 */
UCLASS()
class AGORA_API UWaitDied : public UAgoraBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintAssignable)
    FWaitDiedSignature OnDied;

    UWaitDied();

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitDied* WaitDied(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UBlueprintAsyncActionBase interface

    UFUNCTION()
    void NotifyDied(const FDamageContext& KillingBlow);


private:

   
    IDamageReceiverInterface* ObservedActorAsDamageReceiverInterface;
    TWeakObjectPtr<AActor> ObservedActor;

    FScriptDelegate DiedDelegate;

};
