// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AgoraTypes.h"
#include "AgoraGameMode.generated.h"

namespace MatchState
{
	extern AGORA_API const FName BeginGameCountdown;			// All players have loaded in, and we are counting down to begin the game
	extern AGORA_API const FName GameBegun;						// All players have loaded in, and we are done counting down
}

class UAgoraTowersStateComponent;
class AController;

UCLASS()
class AGORA_API AAgoraGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	AAgoraGameMode();

	// Begin AGameMode/AGameModeBase
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;
	
	// Not used in packaged game, just for PIE
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController);
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	bool CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget) override;
	// ~End AGameMode/AGameModeBase

	UPROPERTY(VisibleAnywhere)
	UAgoraTowersStateComponent* TowersState = nullptr;

    UFUNCTION(BlueprintCallable, Category = "AgoraGameMode")
    virtual void NotifyKilled(AActor* Killer, AActor* Killed, FDamageContext DamageContext);

	UFUNCTION(BlueprintCallable)
	void FinishMatch(APawn* DestroyedCore);

	UFUNCTION(BlueprintPure, Category = "AgoraGameMode")
	AAgoraGameState* GetAgoraGameState();

	void NotifyClientLoaded();

protected:
	float NumClientsLoaded = 0;

	bool AllClientsConnected();

	void HandleConnectPlayer();

	UFUNCTION()
	void HandleBeginCountdown();

	/**
	 * This is the match state for after the countdown finishes
	 */
	UFUNCTION()
	void HandleBeginGame();

	UPROPERTY(EditDefaultsOnly, Category = "AgoraGameMode")
	float MatchStartCountdownDuration = 10.f;
};
