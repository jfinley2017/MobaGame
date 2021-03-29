// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraBlueprintAsyncActionBase.h"
#include "AgoraTypes.h"
#include "WaitDamageTaken.generated.h"

class IDamageReceiverInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitDamageTakenSignature, const FDamageContext&, Damage);

/**
 * Fires when the provided actor takes damage
 */
UCLASS()
class AGORA_API UWaitDamageTaken : public UAgoraBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintAssignable)
    FWaitDamageTakenSignature OnDamageTaken;

    UWaitDamageTaken(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, meta = (LatentInfo = "InLatentActionInfo", WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", UnsafeDuringActorConstruction), Category = "AgoraAsyncLibrary")
    static UWaitDamageTaken* WaitDamageTaken(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor);

    // UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void Cleanup() override;
    // ~UBlueprintAsyncActionBase interface

    UFUNCTION()
    void NotifyDamageTaken(float CurrentHP, float MaxHP, const FDamageContext& Damage);


private:
 
    IDamageReceiverInterface* ObservedActorAsDamageReceiverInterface;

    FScriptDelegate DamageTakenDelegate;
};
