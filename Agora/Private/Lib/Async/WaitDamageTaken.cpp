// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "WaitDamageTaken.h"
#include "Interfaces/DamageReceiverInterface.h"

UWaitDamageTaken::UWaitDamageTaken(const FObjectInitializer& ObjectInitializer)
{
    DamageTakenDelegate.BindUFunction(this, "NotifyDamageTaken");
}

UWaitDamageTaken* UWaitDamageTaken::WaitDamageTaken(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo, AActor* ObservedActor)
{
    UWaitDamageTaken* BlueprintNode = NewAgoraAsyncTask<UWaitDamageTaken>(WorldContextObject, InLatentActionInfo);
    BlueprintNode->ObservedActorAsDamageReceiverInterface = Cast<IDamageReceiverInterface>(ObservedActor);
    // #TODO: Maybe check if we're not testing so we can check this
	// ensure(BlueprintNode->ObservedActorAsDamageReceiverInterface);
    return BlueprintNode;
}

void UWaitDamageTaken::Activate()
{
    if (ObservedActorAsDamageReceiverInterface)
    {
        ObservedActorAsDamageReceiverInterface->GetDamageReceivedDelegate().AddUnique(DamageTakenDelegate);
    }

    Super::Activate();
}


void UWaitDamageTaken::Cleanup()
{

    if (ObservedActorAsDamageReceiverInterface)
    {
        ObservedActorAsDamageReceiverInterface->GetDamageReceivedDelegate().Remove(DamageTakenDelegate);
    }

    Super::Cleanup();
}

void UWaitDamageTaken::NotifyDamageTaken(float CurrentHP, float MaxHP, const FDamageContext& Damage)
{
	if (Damage.IsValid())
	{
		OnDamageTaken.Broadcast(Damage);
	}
}
