// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "WaitDied.h"
#include "DamageReceiverInterface.h"

UWaitDied::UWaitDied()
{
    DiedDelegate.BindUFunction(this, "NotifyDied");
}

UWaitDied* UWaitDied::WaitDied(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor)
{
    UWaitDied* BlueprintNode = NewAgoraAsyncTask<UWaitDied>(WorldContextObject, InLatentActionInfo);
    BlueprintNode->ObservedActor = ObservedActor;
    BlueprintNode->ObservedActorAsDamageReceiverInterface = Cast<IDamageReceiverInterface>(ObservedActor);
    if (!BlueprintNode->ObservedActorAsDamageReceiverInterface)
    {
        TRACESTATIC(LogAgoraBlueprintAsyncActionBase, Warning, 
              "%s does not implement IDamageReceiverInterface. %s -> %s will never be called.", *GetNameSafe(ObservedActor), *GetNameSafe(BlueprintNode), *GetNameSafe(InLatentActionInfo.CallbackTarget));
    }
    return BlueprintNode;
}

void UWaitDied::Activate()
{
    Super::Activate();

    if (ObservedActor.Get() && ObservedActorAsDamageReceiverInterface)
    {
        ObservedActorAsDamageReceiverInterface->GetDeathDelegate().AddUnique(DiedDelegate);
    }
    

}

void UWaitDied::Cleanup()
{

    if (ObservedActor.Get() && ObservedActorAsDamageReceiverInterface)
    {
        ObservedActorAsDamageReceiverInterface->GetDeathDelegate().RemoveAll(this);
    }

    Super::Cleanup();
}

void UWaitDied::NotifyDied(const FDamageContext& KillingBlow)
{
    OnDied.Broadcast(ObservedActor.Get(),KillingBlow);
}
