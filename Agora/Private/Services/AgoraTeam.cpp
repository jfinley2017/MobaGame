// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraTeam.h"
#include "AgoraHeroBase.h"
#include "AgoraMinionBase.h"
#include "AgoraTeamLibrary.h"
#include "Agora.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayActors/AgoraTeamActor.h"

AAgoraTeam::AAgoraTeam()
{
    SetReplicates(true);
    bAlwaysRelevant = true;
}

void AAgoraTeam::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAgoraTeam, TeamID);
    DOREPLIFETIME(AAgoraTeam, Heroes);
    DOREPLIFETIME(AAgoraTeam, Minions);
    DOREPLIFETIME(AAgoraTeam, MiscActors);
    DOREPLIFETIME(AAgoraTeam, Players);
}

void AAgoraTeam::SetTeamID(ETeam NewTeamID)
{
    TeamID = NewTeamID;
}

void AAgoraTeam::BeginPlay()
{
    Super::BeginPlay();

    TRACE(AgoraTeam, Log, "Team %s BeginPlay.", *UAgoraTeamLibrary::GetTeamNameFromID(TeamID));
}

void AAgoraTeam::NotifyHeroJoinedTeam(AAgoraHeroBase* Hero)
{
    if (!Heroes.Contains(Hero))
    {
        Heroes.Add(Hero);
        OnHeroJoinedTeam.Broadcast(TeamID, Hero);
    }
}

void AAgoraTeam::NotifyHeroLeftTeam(AAgoraHeroBase* Hero)
{
    if (Heroes.Remove(Hero))
    {
        OnHeroLeftTeam.Broadcast(TeamID, Hero);
    }
}

void AAgoraTeam::NotifyMinionJoinedTeam(AAgoraMinionBase* Minion)
{
    if (!Minions.Contains(Minion))
    {
        Minions.Add(Minion);
        OnMinionJoinedTeam.Broadcast(TeamID, Minion);
    }
    
}

void AAgoraTeam::NotifyMinionLeftTeam(AAgoraMinionBase* Minion)
{
    if (Minions.Remove(Minion))
    {
        OnMinionLeftTeam.Broadcast(TeamID, Minion);
    }
}

void AAgoraTeam::NotifyPlayerJoinedTeam(APlayerState* Player)
{
    if (!Players.Contains(Player))
    {
        Players.Add(Player);
        OnPlayerJoinedTeam.Broadcast(TeamID, Player);
        ClientNotifyPlayerJoinedTeam(Player);
    }
    
}

void AAgoraTeam::NotifyPlayerLeftTeam(APlayerState* Player)
{
    if (Players.Remove(Player))
    {
        OnPlayerLeftTeam.Broadcast(TeamID, Player);
        ClientNotifyPlayerLeftTeam(Player);
    }
}

void AAgoraTeam::NotifyMiscActorJoinedTeam(AActor* Actor)
{
    if (!MiscActors.Contains(Actor))
    {
        MiscActors.Add(Actor);
        OnMiscActorJoinedTeam.Broadcast(TeamID, Actor);
    }
    
}

void AAgoraTeam::NotifyMiscActorLeftTeam(AActor* Actor)
{
    if (MiscActors.Remove(Actor))
    {
        OnMiscActorLeftTeam.Broadcast(TeamID, Actor);
    }
}

void AAgoraTeam::GetAllActorsOnTeam(TArray<AActor*>& OutActors)
{
    OutActors.Append(Heroes);
    OutActors.Append(Minions);
    OutActors.Append(MiscActors);
}

void AAgoraTeam::GetHeroesOnTeam(TArray<AAgoraHeroBase*>& OutHeroes)
{
    OutHeroes = Heroes;
}

void AAgoraTeam::GetPlayersOnTeam(TArray<APlayerState*>& OutPlayers)
{
    OutPlayers = Players;
}

void AAgoraTeam::GetMinionsOnTeam(TArray<AAgoraMinionBase*>& OutMinions)
{
    OutMinions = Minions;
}

void AAgoraTeam::GetViewTargetsOnTeam(TArray<AActor*>& OutViewTargets)
{
	// We don't expect this to ever change at the moment, so we lazily use get all actors of class and cache the result
	if (ViewTargets.Num() == 0)
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAgoraTeamActor::StaticClass(), ViewTargets);
		ViewTargets = ViewTargets.FilterByPredicate([](AActor* Actor) { return Actor->ActorHasTag("TeamViewPoint"); });
	}

	OutViewTargets = ViewTargets;
}

ETeam AAgoraTeam::GetTeamID()
{
    return TeamID;
}

void AAgoraTeam::ClientNotifyPlayerJoinedTeam_Implementation(APlayerState* Player)
{
    OnPlayerJoinedTeam.Broadcast(GetTeamID(), Player);
}

void AAgoraTeam::ClientNotifyPlayerLeftTeam_Implementation(APlayerState* Player)
{
    OnPlayerLeftTeam.Broadcast(GetTeamID(), Player);
}
