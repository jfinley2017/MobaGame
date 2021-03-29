//// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
//
//
#include "AgoraMoveComponentTo.h"
#include "Agora.h"
#include "Engine.h"
#include "Engine/LatentActionManager.h"

void UAgoraMoveComponentTo::_Completed()
{
	Completed.Broadcast();
}
UAgoraMoveComponentTo* UAgoraMoveComponentTo::MoveComponentTo(UObject* WorldContextObject, USceneComponent* Component, FVector TargetRelativeLocation, FRotator TargetRelativeRotation, bool bEaseIn, bool bEaseOut, float OverTime, FLatentActionInfo LatentInfo)
{
	UAgoraMoveComponentTo* MoveComponent = NewObject<UAgoraMoveComponentTo>();
	MoveComponent->WorldContextObject = WorldContextObject;
	MoveComponent->Component = Component;
	MoveComponent->TargetRelativeLocation = TargetRelativeLocation;
	MoveComponent->TargetRelativeRotation = TargetRelativeRotation;
	MoveComponent->bEaseIn = bEaseIn;
	MoveComponent->bEaseOut = bEaseOut;
	MoveComponent->OverTime = OverTime;
	MoveComponent->LatentInfo = LatentInfo;
	MoveComponent->LatentInfo.CallbackTarget = WorldContextObject;
	return MoveComponent;
}

void UAgoraMoveComponentTo::Activate()
{
	if (!WorldContextObject)
	{
		TRACESTATIC(Agora, Fatal, "World context object in AgoraMoveComponentTo is null!");
	}

	//if this guy fires off while the component is already moving, we want to cancel the current movement and begin a new movement.
	//so we kill this guy's existing latent actions.
	//note that this would kill ALL delays in progress. If that gets to be an issue then I'll replace this with a better solution.
	WorldContextObject->GetWorld()->GetLatentActionManager().RemoveActionsForObject(WorldContextObject);

	UKismetSystemLibrary::MoveComponentTo(Component, TargetRelativeLocation, TargetRelativeRotation, bEaseOut, bEaseIn, OverTime, false, EMoveComponentAction::Move, LatentInfo);
	FTimerHandle TimerHandle;
	WorldContextObject->GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UAgoraMoveComponentTo::_Completed, OverTime, false);
}