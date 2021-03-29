// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraMapLibrary.h"
#include "GameFramework/Actor.h"
#include "AgoraGameState.h"
#include "MinimapDisplayableInterface.h"
#include "AgoraMapIconComponent.h"
#include "AgoraMapTrackerService.h"

DEFINE_LOG_CATEGORY(AgoraMap);

AAgoraMapTrackerService* UAgoraMapLibrary::GetMapTrackerService(UObject* WorldContextObject)
{
    if (!WorldContextObject || !WorldContextObject->GetWorld()) { return nullptr; }
    if (WorldContextObject->GetWorld()->GetNetMode() == NM_DedicatedServer) { return nullptr; }

    AAgoraGameState* AgoraGameState = WorldContextObject->GetWorld()->GetGameState<AAgoraGameState>();
    if (AgoraGameState)
    {
        return AgoraGameState->GetMapTrackerService();
    }

    return nullptr;
}

void UAgoraMapLibrary::GetTrackedActors(UObject* WorldContextObject, TArray<AActor*>& OutTrackedActors)
{
    AAgoraMapTrackerService* MapTrackerService = GetMapTrackerService(WorldContextObject);
    MapTrackerService->GetTrackedActors(OutTrackedActors);
}

UAgoraMapIconComponent* UAgoraMapLibrary::GetMapIconComponent(AActor* Actor)
{
    IMinimapDisplayableInterface* ActorAsMinimapDisplayableInterface = Cast<IMinimapDisplayableInterface>(Actor);
    if (ActorAsMinimapDisplayableInterface)
    {
        return ActorAsMinimapDisplayableInterface->GetMapIconComponent();
    }

    TRACESTATIC(AgoraMap, Warning, "%s did not implement IMinimapDisplayableInterface. Searching for component through component list.", *GetNameSafe(Actor));
    return Actor->FindComponentByClass<UAgoraMapIconComponent>();
}
