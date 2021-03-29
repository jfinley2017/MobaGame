#include "AgoraLobbyController.h"
#include "AgoraCharacterBase.h"
#include "AgoraPlayerState.h"
#include "AgoraLobbyState.h"
#include "AgoraLobbyMode.h"

void AAgoraLobbyController::OnSelectHero(TSubclassOf<AAgoraCharacterBase> HeroSelected)
{
	AAgoraPlayerState* PS = GetPlayerState<AAgoraPlayerState>();
	check(PS);

	PS->SetHeroClass(HeroSelected);
}

void AAgoraLobbyController::StartGame()
{
	ServerStartGame();
}

void AAgoraLobbyController::BeginPlay()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AAgoraLobbyController::ServerStartGame_Implementation()
{
	AAgoraLobbyMode* GM = GetWorld()->GetAuthGameMode<AAgoraLobbyMode>();
	check(GM);
	GM->StartGame();
}

bool AAgoraLobbyController::ServerStartGame_Validate()
{
	return true;
}
