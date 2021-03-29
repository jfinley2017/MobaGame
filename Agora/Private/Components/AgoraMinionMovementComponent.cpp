// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraMinionMovementComponent.h"

UAgoraMinionMovementComponent::UAgoraMinionMovementComponent(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    NetworkSmoothingMode = ENetworkSmoothingMode::Linear;
}

void UAgoraMinionMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    if (!GetOwner()->HasAuthority())
    {
        if (CharacterOwner->GetMesh()->bRecentlyRendered)
        {
            PrimaryComponentTick.TickInterval = 0.0f;
            SimulatedTick(DeltaTime);
        }
        else
        {
            PrimaryComponentTick.TickInterval = .1f;
            SmoothClientPosition(DeltaTime);
        }
        return;
    }

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
