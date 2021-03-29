// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AgoraLobbyMode.generated.h"

class APlayerController;
class AAgoraCharacterBase;
class AAgoraPlayerState;

/**
 * Gamemode for lobbies, could potentially be converted to a component that is attached to other game modes
 * Component would be useful if you wanted a GM that did non lobby things. Until then, this provides a cleaner interface
 *
 * GM shouldn't need to care about whether players or AI are drafting, the AI controller can be extended to handle drafting via events
 * It does need to know how many players are involved, and there are different ways we might want to draft
 * Blind pick, draft with/without bans, etc.
 * Additionally for a practice mode you may want to be able to select the class of your enemies as well somehow
 * The most basic version is non-blind pick, we'll do that first
 * GM handles enforcing the time limit. Clients can predict the time on their own
 * Any client disconnect will abort the draft..unless it's a custom game?
 *
 * GM might need to spawn more AI controllers for missing humans, this may need to be read from game instance
 */
UCLASS()
class AGORA_API AAgoraLobbyMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	uint8 NumHumans = 5;

	AAgoraLobbyMode();

	UFUNCTION(BlueprintCallable)
	AAgoraLobbyState* GetLobbyState();

	UFUNCTION()
	void WaitForAllPlayersReady(bool bIsReady, AAgoraPlayerState* APS);

	virtual void BeginPlay() override;

	// Might want to allow fewer players in a practice mode or something
	static const uint8 NumPlayersPerTeam = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	FString LevelName = "OneLane_GreyBox";

	UFUNCTION(BlueprintCallable)
	void StartGame();

	// Selects a hero for the given player - this should be able to handle AI OR humans
	// Various other functions will be needed for locking in a hero
	void SelectHero(TSubclassOf<AAgoraCharacterBase> HeroClass, AAgoraPlayerState* PS);

	// Server needs to wait for everyone to be connected before showing the UI and starting the draft timer
	void OnAllPlayersReadyToDraft();

	// Returns true if any AI were spawned
	bool SpawnAIForMissingHumans();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};
