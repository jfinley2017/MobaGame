// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraTargetingReticle.h"

void AAgoraTargetingReticle::Tick(float DeltaSeconds)
{
	// We specifically want to avoid ticking the parent reticle
	AActor::Tick(DeltaSeconds);
	FRotator TargetRot = TargetingActor->GetActorRotation();
	SetActorRotation(FRotator(0, TargetRot.Yaw, 0));
}

void AAgoraTargetingReticle::InitializeRadius(float InRadius)
{
	Radius = InRadius;
}
