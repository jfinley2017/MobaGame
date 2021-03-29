// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AgoraTypes.h" // FDamageContext
#include "AgoraGameState.generated.h"

class AAgoraPlayerState;
class AAgoraVisionService;
class AAgoraTeamService;
class AAgoraItemService;
class AAgoraMapTrackerService;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisionServiceReadySignature, AAgoraVisionService*, VisionService);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamServiceReadySignature, AAgoraTeamService*, TeamService);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemServiceReadySignature, AAgoraItemService*, ItemService);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveKilledSignature, AActor*, Died, FDamageContext, KillingBlowContext);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNumPlayersChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchStateChanged, FName, State);

/**
 * comment me
 */
UCLASS()
class AGORA_API AAgoraGameState : public AGameState
{
	GENERATED_BODY()
	
public:

    /**
     * This is called after all players have loaded into the game
     */
    UPROPERTY(BlueprintAssignable)
    FOnMatchStateChanged OnMatchStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnNumPlayersChanged OnNumPlayersChanged;

    UPROPERTY(BlueprintAssignable)
    FOnVisionServiceReadySignature OnVisionServiceReady;
    UPROPERTY(BlueprintAssignable)
    FOnTeamServiceReadySignature OnTeamServiceReady;
    UPROPERTY(BlueprintAssignable)
    FOnItemServiceReadySignature OnItemServiceReady;

    UPROPERTY(BlueprintAssignable)
    FOnObjectiveKilledSignature OnTowerKilled;
    UPROPERTY(BlueprintAssignable)
    FOnObjectiveKilledSignature OnHeroKilled;
    UPROPERTY(BlueprintAssignable)
    FOnObjectiveKilledSignature CoreKilled;

    // AGameState interface
    virtual void PreInitializeComponents() override;
	virtual void HandleMatchHasStarted() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
    // ~AGameState interface

	/**
	 * This is the number of players in the game
	 */
	UFUNCTION(BlueprintPure, Category="Agora|AgoraGameState")
	int32 GetNumConnectedPlayers();

	/**
	 * The amount of players that should be in the game once it's fully loaded
	 */
	UFUNCTION(BlueprintPure, Category = "Agora|AgoraGameState")
	int32 GetNumMaxPlayers();

	UFUNCTION(Server, Reliable, WithValidation)
	void NotifyPlayersChanged(int32 InNumTravellingPlayers, int32 InNumConnectedPlayers);

	UFUNCTION()
	void HandlePlayersChanged();

	UFUNCTION(BlueprintCallable, Category="AgoraGameState")
	void GetAgoraPlayers(TArray<AAgoraPlayerState*>& Arr);

	UFUNCTION(BlueprintCallable, Category = "AgoraGameState")
	void GetTeamPlayerStates(ETeam Team, TArray<AAgoraPlayerState*>& OutArr);

    /**
     * Broadcasts to all clients and server that the match has ended. 
     * @Param DestroyedCore pawn that destroyed the core
     */
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_OnMatchComplete(APawn* DestroyedCore);

    /**
     * Broadcasts to all clients and server that a hero was killed
     * @Param Hero the killed hero
     * @Param KillingBlow the damage context that killed the hero
     */
    UFUNCTION(NetMulticast, Reliable)
    void NetMulticast_HeroKilled(AActor* Hero, FDamageContext KillingBlow);

    /**
     * Broadcasts to all clients and server that a tower was killed
     * @Param Tower the destroyed tower
     * @Param KillingBlow the damage context that killed the tower
     */
    UFUNCTION(NetMulticast, Reliable)
    void NetMulticast_TowerDestroyed(AActor* Tower, FDamageContext KillingBlow);

    /**
     * Broadcasts to all clients and the server that the core was killed and the game has ended.
     * @Param Core the destroyed core
     * @Param KillingBlow the damage context that killed the core
     */
    UFUNCTION(NetMulticast, Reliable)
    void NetMulticast_CoreDestroyed(AActor* Core, FDamageContext KillingBlow);
  
	/**
	 * Causes all connected clients to disconnect.
	 * Only run on server.
	 */
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_Disconnect();

	//eventually we'll have the server send the players to a PostGameMenu or something like that. For now just send them back to the main menu.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AgoraGameState")
	FString PostMatchLevelName = "MainMenu";

	UFUNCTION(BlueprintPure, Category = "Agora|AgoraGameState")
	float GetTimeSinceCountdownStarted();

	UFUNCTION(BlueprintPure, Category = "Agora|AgoraGameState")
	float GetSecondsSinceMatchStart();

    AAgoraVisionService* GetVisionService() const;
    AAgoraMapTrackerService* GetMapTrackerService();
    AAgoraTeamService* GetTeamService() const;
    AAgoraItemService* GetItemService() const;

protected:

    // AGameState
    virtual void OnRep_MatchState() override;
    //~AGameState
	
	UPROPERTY(ReplicatedUsing = OnRep_NumConnectedPlayers)
	int32 NumConnectedPlayers = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_NumTravellingPlayers)
	int32 NumTravellingPlayers = 0;

	UPROPERTY(Replicated)
	float TimeMatchBegan = 0;

	UPROPERTY(Replicated)
	float TimeCountdownBegan = 0;

    UPROPERTY(ReplicatedUsing = OnRep_TeamService)
    AAgoraTeamService* TeamService = nullptr;
    UPROPERTY(ReplicatedUsing = OnRep_ItemService)
    AAgoraItemService* ItemService = nullptr;
    UPROPERTY(ReplicatedUsing = OnRep_VisionService)
    AAgoraVisionService* VisionService = nullptr;

    AAgoraMapTrackerService* MapTrackerService;
    
    UFUNCTION()
    virtual void OnRep_TeamService();

    UFUNCTION()
    virtual void OnRep_ItemService();

    UFUNCTION()
    virtual void OnRep_VisionService();

    UFUNCTION()
    void OnRep_NumConnectedPlayers();

    UFUNCTION()
    void OnRep_NumTravellingPlayers();

};
