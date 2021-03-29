// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraVisionLibrary.h"
#include "AgoraGameState.h"
#include "AgoraBlueprintFunctionLibrary.h"

DEFINE_LOG_CATEGORY(AgoraVision);

AAgoraVisionService* UAgoraVisionLibrary::GetVisionService(UObject* WorldContext)
{
    if (!WorldContext->GetWorld()) { return nullptr; }

    AAgoraGameState* AgoraGameState = WorldContext->GetWorld()->GetGameState<AAgoraGameState>();
    if (AgoraGameState)
    {
        return AgoraGameState->GetVisionService();
    }

    static bool AgoraVisionNotFound = true;
    if (AgoraVisionNotFound)
    {
        TRACESTATIC(AgoraVision, Error, "AgoraVisionService not found.");
        AgoraVisionNotFound = false;
    }

    return nullptr;
}

bool UAgoraVisionLibrary::HasVisionOf(UObject* WorldContextObject, const AActor* Viewer, const AActor* Subject)
{
    AAgoraVisionService* VisionService = GetVisionService(WorldContextObject);
    if (VisionService)
    {
        return VisionService->IsVisibleFor(Viewer, Subject);
    }
    return false;
}

bool UAgoraVisionLibrary::LocalPlayerHasVisionOf(UObject* WorldContextObject, const AActor* Subject)
{
    APlayerController* LocalPlayer = UAgoraBlueprintFunctionLibrary::GetViewingPlayerController(WorldContextObject);
    if (LocalPlayer)
    {
        return HasVisionOf(WorldContextObject, LocalPlayer, Subject);
    }
    return false;
}

void UAgoraVisionLibrary::GenerateVisionDebugString(UObject* WorldContextObject, FString& OutDebugString)
{
    AAgoraVisionService* VisionService = UAgoraVisionLibrary::GetVisionService(WorldContextObject);
    if (VisionService)
    {
        OutDebugString = "";
        VisionService->GenerateDebugString(OutDebugString);
    }
}
