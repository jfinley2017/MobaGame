// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#include "AgoraLobbyMode.h"
#include "AgoraGameInstance.h"
#include "AgoraCharacterBase.h"
#include "GameFramework/Controller.h"
#include "AgoraPlayerState.h"
#include "AgoraLobbyState.h"

AAgoraLobbyMode::AAgoraLobbyMode()
{
	GameStateClass = AAgoraLobbyState::StaticClass();
}

// Helper to avoid having to type out the template, and for BP
AAgoraLobbyState* AAgoraLobbyMode::GetLobbyState()
{
	return GetGameState<AAgoraLobbyState>();
}

void AAgoraLobbyMode::WaitForAllPlayersReady(bool bIsReady, AAgoraPlayerState* APS)
{
	bool bAllPlayersReady = true;
	TArray<AAgoraPlayerState*> PSArr;
	GetLobbyState()->GetAgoraPlayers(PSArr);

	for (AAgoraPlayerState* LoopPlayer : PSArr)
	{
		if (!LoopPlayer->GetPlayerData().bIsReadyInLobby) 
		{
			bAllPlayersReady = false;
		}
	}

	if (bAllPlayersReady)
	{
		GetLobbyState()->Multicast_AllPlayersReady();
		StartGame();
	}
}

void AAgoraLobbyMode::BeginPlay()
{
	OnAllPlayersReadyToDraft();

	GetLobbyState()->PlayerReadyDel.AddDynamic(this, &AAgoraLobbyMode::WaitForAllPlayersReady);
}

void AAgoraLobbyMode::StartGame()
{
	GetWorld()->ServerTravel(LevelName + "?listen");
}

void AAgoraLobbyMode::OnAllPlayersReadyToDraft()
{
	SpawnAIForMissingHumans();
	AAgoraLobbyState* GS = GetLobbyState();
}

bool AAgoraLobbyMode::SpawnAIForMissingHumans()
{
	TArray<APlayerState*> PlayerArr = GetGameState<AAgoraLobbyState>()->PlayerArray;
	//while (PlayerArr.Num() < NumPlayersPerTeam)
	//{
	//	// will this work? We'll find out I guess
	//	//GetWorld()->SpawnActor<AAgoraAILobbyController>();
	//}
	return true;
}

void AAgoraLobbyMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void AAgoraLobbyMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}
