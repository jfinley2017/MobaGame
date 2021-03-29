// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraMapTrackerService.h"

void AAgoraMapTrackerService::RegisterWithTracker(AActor* Actor)
{
    TrackedMapActors.Add(Actor);
    OnActorRegistered.Broadcast(Actor);
}

void AAgoraMapTrackerService::UnRegisterWithTracker(AActor* Actor)
{
    TrackedMapActors.Remove(Actor);
    OnActorUnRegistered.Broadcast(Actor);
}

void AAgoraMapTrackerService::GetTrackedActors(TArray<AActor*>& Actors)
{
    Actors.Append(TrackedMapActors.Array());
}
