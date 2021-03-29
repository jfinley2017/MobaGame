// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraTeamLibrary.h"
#include "AgoraGameState.h"
#include "AgoraTeamService.h"
#include "AgoraTeam.h"
#include "AgoraBlueprintFunctionLibrary.h"

DEFINE_LOG_CATEGORY(AgoraTeam);

AAgoraTeamService* UAgoraTeamLibrary::GetAgoraTeamService(UObject* WorldContextObject)
{
    if (!WorldContextObject->GetWorld()) { return nullptr; }

    AAgoraGameState* AgoraGameState = WorldContextObject->GetWorld()->GetGameState<AAgoraGameState>();
    if (AgoraGameState)
    {
        return AgoraGameState->GetTeamService();
    }

    static bool AgoraTeamNotFound = true;
    if (AgoraTeamNotFound)
    {
        TRACESTATIC(AgoraTeam, Error, "AgoraTeamService not found.");
        AgoraTeamNotFound = false;
    }

    return nullptr;
}

AAgoraTeam* UAgoraTeamLibrary::GetTeamByID(UObject* WorldContextObject, ETeam TeamID)
{
    AAgoraTeamService* TeamService = GetAgoraTeamService(WorldContextObject);

    if (TeamService)
    {
        return TeamService->GetTeamByID(TeamID);
    }
    return nullptr;
}

FString UAgoraTeamLibrary::GetTeamNameFromID(ETeam TeamID)
{
    return GETENUMSTRING("ETeam", TeamID);
}

ETeam UAgoraTeamLibrary::GetTeam(AActor* Actor)
{
    ITeamInterface* ActorAsTeamInterface = Cast<ITeamInterface>(Actor);
    if (ActorAsTeamInterface)
    {
        return ActorAsTeamInterface->GetTeam();
    }
    return ETeam::Invalid;
}

void UAgoraTeamLibrary::GenerateTeamServiceDebugString(UObject* WorldContextObject, FString& OutDebugString)
{
    AAgoraTeamService* TeamService = GetAgoraTeamService(WorldContextObject);

    if (!TeamService) 
    { 
        OutDebugString = "No TeamService available";
        return;  
    }

    OutDebugString = "";
    TeamService->GenerateDebugString(OutDebugString);
}

bool UAgoraTeamLibrary::IsFriendly(const AActor* ActorOne, const AActor* ActorTwo)
{
    if (!ActorOne && !ActorTwo)
    {
        return false;
    }

    if (!ActorOne || !ActorTwo)
    {
        UWorld* WorldContext = ActorOne ? ActorOne->GetWorld() : ActorTwo->GetWorld();
        if (WorldContext->WorldType != EWorldType::Editor)
        {
            TRACESTATIC(AgoraTeam, Warning, "Null actor found during IsFriendly between %s and %s.", *GetNameSafe(ActorOne), *GetNameSafe(ActorTwo));
        }
        return false;
    }

    const ITeamInterface* ActorOneAsTeamInterface = Cast<ITeamInterface>(ActorOne);
    const ITeamInterface* ActorTwoAsTeamInterface = Cast<ITeamInterface>(ActorTwo);

    if (ActorOneAsTeamInterface && ActorTwoAsTeamInterface)
    {
        ETeam ActorOneTeam = ActorOneAsTeamInterface->GetTeam();
        ETeam ActorTwoTeam = ActorTwoAsTeamInterface->GetTeam();

        if (ActorOneTeam == ETeam::Invalid)
        {
            TRACESTATIC(AgoraTeam, Warning, "%s Invalid Team", *GetNameSafe(ActorOne));
        }
        if (ActorTwoTeam == ETeam::Invalid)
        {
            TRACESTATIC(AgoraTeam, Warning, "%s Invalid Team", *GetNameSafe(ActorTwo));
        }

        return ActorOneTeam == ActorTwoTeam;
    }


    FString ErrorNames;
    if (!ActorOneAsTeamInterface)
    {
        ErrorNames += GetNameSafe(ActorOne) + " ";
    }
    if (!ActorTwoAsTeamInterface)
    {
        ErrorNames += GetNameSafe(ActorTwo) + " ";
    }

    TRACESTATIC(AgoraTeam, Warning, "% failed to implement ITeamInterface.", *ErrorNames);
    return false;
}

void UAgoraTeamLibrary::GetActorsOnTeam(UObject* WorldContextObject, ETeam TeamID, TArray<AActor*>& Actors)
{
    AAgoraTeamService* TeamService = GetAgoraTeamService(WorldContextObject);
    if (TeamService)
    {
        TeamService->GetActorsOnTeam(TeamID, Actors);
    }
}

void UAgoraTeamLibrary::GetHeroesOnTeam(UObject* WorldContextObject, ETeam TeamID, TArray<AAgoraHeroBase*>& Heroes)
{
    AAgoraTeamService* TeamService = GetAgoraTeamService(WorldContextObject);
    if (TeamService)
    {
        TeamService->GetHeroesOnTeam(TeamID, Heroes);
    }
}

void UAgoraTeamLibrary::GetPlayersOnTeam(UObject* WorldContextObject, ETeam TeamID, TArray<APlayerState*>& OutPlayersForTeam)
{
    AAgoraTeamService* TeamService = GetAgoraTeamService(WorldContextObject);
    if (TeamService)
    {
        TeamService->GetPlayersOnTeam(TeamID, OutPlayersForTeam);
    }
}

EOwnerRelationship UAgoraTeamLibrary::DetermineRelationshipToLocalPlayer(UObject* WorldContextObject, AActor* Actor)
{
    APlayerController* LocalPlayerController = UAgoraBlueprintFunctionLibrary::GetViewingPlayerController(WorldContextObject);

    if (!Actor || !LocalPlayerController)
    {
        return EOwnerRelationship::None;
    }

    if (UAgoraGameplayTags::IsSpectator(LocalPlayerController))
    {
        return EOwnerRelationship::Spectator;
    }

    if (Actor == LocalPlayerController->GetPawn())
    {
        return EOwnerRelationship::ObservedOwner;
    }

    bool bIsFriendly = UAgoraTeamLibrary::IsFriendly(LocalPlayerController, Actor);

    if (!bIsFriendly)
    {
        return EOwnerRelationship::Enemy;
    }

    if (bIsFriendly)
    {
        return EOwnerRelationship::Friendly;
    }

    return EOwnerRelationship::None;
}
