// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#include "AgoraLobbyState.h"
#include "AgoraLobbyMode.h"
#include "AgoraPlayerState.h"

void AAgoraLobbyState::NotifyPlayerTeamChanged(AActor* PlayerState)
{
    AAgoraPlayerState* ChangedPlayer = Cast<AAgoraPlayerState>(PlayerState);
    if (ChangedPlayer)
    {
        TeamChangedDel.Broadcast(ChangedPlayer->GetTeam(), ChangedPlayer);
    }
}

void AAgoraLobbyState::GetAgoraPlayers(TArray<AAgoraPlayerState*>& Arr)
{
	Arr.Empty();

	for (APlayerState* PS : PlayerArray)
	{
		AAgoraPlayerState* APS = Cast<AAgoraPlayerState>(PS);
		Arr.Add(APS);
	}
}

void AAgoraLobbyState::Multicast_AllPlayersReady_Implementation()
{
	AllPlayersReadyDel.Broadcast();
}

bool AAgoraLobbyState::Multicast_AllPlayersReady_Validate()
{
	return true;
}

AAgoraLobbyMode* AAgoraLobbyState::GetAuthorityLobbyMode()
{
	if (!AuthorityGameMode) { return nullptr; }
	
	AAgoraLobbyMode* AGM = Cast<AAgoraLobbyMode>(AuthorityGameMode);

	check(AGM);

	return Cast<AAgoraLobbyMode>(AGM);
}

void AAgoraLobbyState::StartGame()
{
	GetAuthorityLobbyMode()->StartGame();
}

void AAgoraLobbyState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	PlayersChangedDel.Broadcast();
}

void AAgoraLobbyState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	PlayersChangedDel.Broadcast();
}
