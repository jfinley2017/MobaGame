// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AgoraCharacterBase.h" // For TSubclassOf
#include "AgoraLobbyState.generated.h"

class AAgoraPlayerState;
class AAgoraLobbyMode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHeroClassChanged, TSubclassOf<AAgoraCharacterBase>, HeroClass, AAgoraPlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamChanged, ETeam, Team, AAgoraPlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FReadyChanged, bool, PlayerIsReady, AAgoraPlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAllPlayersReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayersChanged);

/**
 * For drafting, picking heroes, teams, special abilities
 */
UCLASS()
class AGORA_API AAgoraLobbyState : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintAssignable)
	FPlayersChanged PlayersChangedDel;

	UPROPERTY(BlueprintAssignable)
	FHeroClassChanged HeroClassChangedDel;

	UPROPERTY(BlueprintAssignable)
	FTeamChanged TeamChangedDel;

	UPROPERTY(BlueprintAssignable)
	FReadyChanged PlayerReadyDel;

	UPROPERTY(BlueprintAssignable)
	FAllPlayersReady AllPlayersReadyDel;

    UFUNCTION()
    void NotifyPlayerTeamChanged(AActor* PlayerState);

    UFUNCTION(BlueprintPure)
    void GetAgoraPlayers(TArray<AAgoraPlayerState*>& Arr);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multicast_AllPlayersReady();

	UFUNCTION(BlueprintCallable)
	AAgoraLobbyMode* GetAuthorityLobbyMode();

	UFUNCTION(BlueprintCallable)
	void StartGame();

	/**
	 * Overridden to support player state changed delegate
	 * Originally fired off delegates from the GM and NetMulticast'd them, however this resulted
	 * in a race condition in which the delegates fired before the game states all replicated
	 * so the update would occur too early. Game state is managed independently on client and server
	 * so this requires no replication
	 */
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

};
