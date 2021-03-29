// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraTeamService.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "AgoraTeamLibrary.h"
#include "AgoraTeam.h"
#include "Agora.h"
#include "AgoraHeroBase.h"
#include "AgoraMinionBase.h"
#include "GameFramework/PlayerController.h"
#include "TeamInterface.h"
#include "GameFramework/PlayerState.h"

void AAgoraTeamService::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAgoraTeamService, Teams);
}

AAgoraTeamService::AAgoraTeamService()
{
    SetReplicates(true);
    bAlwaysRelevant = true;
}

void AAgoraTeamService::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    SetupTeams();
}


void AAgoraTeamService::BeginPlay()
{
    Super::BeginPlay();

    TRACE(AgoraTeam, Log, "AgoraTeamService BeginPlay.");

    if (!HasAuthority())
    {
        return;
    }

}

void AAgoraTeamService::SetupTeams()
{
    if (!HasAuthority()) { return; }

    AAgoraTeam* DuskTeam = GetWorld()->SpawnActor<AAgoraTeam>();
    DuskTeam->SetTeamID(ETeam::Dusk);

    AAgoraTeam* DawnTeam = GetWorld()->SpawnActor<AAgoraTeam>();
    DawnTeam->SetTeamID(ETeam::Dawn);

    Teams.Add(DuskTeam);
    Teams.Add(DawnTeam);

}

void AAgoraTeamService::AddActorToTeam(ETeam TeamID, AActor* Actor)
{
    if (!HasAuthority()) { return; }

    AAgoraTeam* Team = GetTeamFromID(TeamID);
    if (!Team) 
    { 
        TRACE(AgoraTeam, Warning, "%s tried joining Team %s, but no team was found with that ID", *GetNameSafe(Actor), *UAgoraTeamLibrary::GetTeamNameFromID(TeamID))
        return; 
    }

    // print this error
    ITeamInterface* ActorAsTeamInterface = Cast<ITeamInterface>(Actor);
    if (!ActorAsTeamInterface) 
    { 
        TRACE(AgoraTeam, Warning, "%s tried joining Team %s, but they did not implement ITeamInterface", *GetNameSafe(Actor), *UAgoraTeamLibrary::GetTeamNameFromID(TeamID))
        return; 
    }

    TRACE(AgoraTeam, Log, "%s joined Team %s", *GetNameSafe(Actor), *UAgoraTeamLibrary::GetTeamNameFromID(TeamID))

    // we should be using tags for this, but for the moment i figured the usefulness of getting types back from teams 
    // was worth it. if we ever manage to get to a pure-ecs solution for our characters, we should change this.
    if (Cast<AAgoraHeroBase>(Actor))
    {
        Team->NotifyHeroJoinedTeam(Cast<AAgoraHeroBase>(Actor));
        return;
    }

    if (Cast<AAgoraMinionBase>(Actor))
    {
        Team->NotifyMinionJoinedTeam(Cast<AAgoraMinionBase>(Actor));
        return;
    }

    if (Cast<APlayerState>(Actor))
    {
        Team->NotifyPlayerJoinedTeam(Cast<APlayerState>(Actor));
        return;
    }

    
    Team->NotifyMiscActorJoinedTeam(Actor);
}

AAgoraTeam* AAgoraTeamService::GetTeamByID(ETeam TeamID)
{
    return GetTeamFromID(TeamID);
}

void AAgoraTeamService::RemoveActorFromTeam(ETeam TeamID, AActor* Actor)
{
    if (!HasAuthority()) { return; }

    AAgoraTeam* Team = GetTeamFromID(TeamID);
    if (!Team) { return; }

    // print this error
    ITeamInterface* ActorAsTeamInterface = Cast<ITeamInterface>(Actor);
    if (!ActorAsTeamInterface) { return; }

    TRACE(AgoraTeam, Log, "%s left Team %s", *GetNameSafe(Actor), *UAgoraTeamLibrary::GetTeamNameFromID(TeamID))

    // we should be using tags for this, but for the moment i figured the usefulness of getting types back from teams 
    // was worth it. if we ever manage to get to a pure-ecs solution for our characters, we should change this.
    if (Cast<AAgoraHeroBase>(Actor))
    {
        Team->NotifyHeroLeftTeam(Cast<AAgoraHeroBase>(Actor));
        return;
    }

    if (Cast<AAgoraMinionBase>(Actor))
    {
        Team->NotifyMinionLeftTeam(Cast<AAgoraMinionBase>(Actor));
        return;
    }

    if (Cast<APlayerState>(Actor))
    {
        Team->NotifyPlayerLeftTeam(Cast<APlayerState>(Actor));
        return;
    }

    Team->NotifyMiscActorLeftTeam(Actor);
    

}

void AAgoraTeamService::GetActorsOnTeam(ETeam TeamID, TArray<AActor*>& OutActors)
{
    AAgoraTeam* Team = GetTeamFromID(TeamID);
    if (!Team) { return; }

    Team->GetAllActorsOnTeam(OutActors);
}

void AAgoraTeamService::GetMinionsOnTeam(ETeam TeamID, TArray<AAgoraMinionBase*>& OutMinions)
{
    AAgoraTeam* Team = GetTeamFromID(TeamID);
    if (!Team) { return; }

    Team->GetMinionsOnTeam(OutMinions);
}

void AAgoraTeamService::GetHeroesOnTeam(ETeam TeamID, TArray<AAgoraHeroBase*>& OutHeroes)
{
    AAgoraTeam* Team = GetTeamFromID(TeamID);
    if (!Team) { return; }

    Team->GetHeroesOnTeam(OutHeroes);
}

void AAgoraTeamService::GetPlayersOnTeam(ETeam TeamID, TArray<APlayerState*>& OutPlayerStates)
{
    AAgoraTeam* Team = GetTeamFromID(TeamID);
    if (!Team) { return; }

    Team->GetPlayersOnTeam(OutPlayerStates);
}

void AAgoraTeamService::GetViewTargetsOnTeam(ETeam TeamID, TArray<AActor*>& ViewTargets)
{
	AAgoraTeam* Team = GetTeamFromID(TeamID);
	if (!Team) { return; }

	Team->GetViewTargetsOnTeam(ViewTargets);
}

void AAgoraTeamService::GenerateDebugString(FString& OutDebugString)
{
    for (AAgoraTeam* Team : Teams)
    {
        TArray<AAgoraHeroBase*> Heroes;
        TArray<AAgoraMinionBase*> Minions;
        TArray<APlayerState*> Players;

        Team->GetHeroesOnTeam(Heroes);
        Team->GetMinionsOnTeam(Minions);
        Team->GetPlayersOnTeam(Players);

        ETeam TeamID = Team->GetTeamID();
        OutDebugString += FString::Printf(TEXT("\n%s:\n"), *UAgoraTeamLibrary::GetTeamNameFromID(TeamID));
        OutDebugString += FString("Heroes: ");
        for (AAgoraHeroBase* Actor : Heroes)
        {
            OutDebugString += *GetNameSafe(Actor) + FString(", ");
        }
        OutDebugString.LeftChop(2);
        OutDebugString += FString("\n");

        OutDebugString += FString("Minions: ");
        for (AAgoraMinionBase* Actor : Minions)
        {
            OutDebugString += *GetNameSafe(Actor) + FString(", ");
        }
        OutDebugString.LeftChop(2);
        OutDebugString += FString("\n");

        OutDebugString += FString("Players: ");
        for (APlayerState* Actor : Players)
        {
            OutDebugString += *GetNameSafe(Actor) + FString(", ");
        }
        OutDebugString.LeftChop(2);
        OutDebugString += FString("\n");

    }
}

AAgoraTeam* AAgoraTeamService::GetTeamFromID(ETeam TeamID)
{
    for (AAgoraTeam* Team : Teams)
    {
        if (Team && Team->GetTeamID() == TeamID)
        {
            return Team;
        }
    }
    // print here
    return nullptr;
}

void AAgoraTeamService::PrintArray(TArray<AActor*>& ArrayToPrint, FString& OutString)
{
    for (AActor* Actor : ArrayToPrint)
    {
        OutString += *GetNameSafe(Actor) + FString(", ");
    }
    OutString.LeftChop(2);
}

