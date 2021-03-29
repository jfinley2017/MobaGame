// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Interfaces/TeamInterface.h"
#include "AgoraTeamService.generated.h"

class AAgoraTeam;
class AAgoraMinionBase;
class AAgoraHeroBase;

/**
 * Handles the grouping of actors into teams. Responsible for holding lists of actors for each team.
 */
UCLASS()
class AGORA_API AAgoraTeamService : public AInfo
{
	GENERATED_BODY()
	
	
public:

    AAgoraTeamService();

    // AInfo interface
    virtual void PreInitializeComponents() override;
    // ~AInfo interface

    /**
     * Returns a team with the specified TeamID, nullptr if no team with that ID
     */
    UFUNCTION()
    AAgoraTeam* GetTeamByID(ETeam TeamID);

    /**
    * Adds an actor to the team specified by TeamID, does nothing if there is no team with the specified TeamID
    */
    UFUNCTION()
    void AddActorToTeam(ETeam TeamID, AActor* Actor);

    /**
    * Removes an actor to the team specified by TeamID, does nothing if there is no team with the specified TeamID
    */
    UFUNCTION()
    void RemoveActorFromTeam(ETeam TeamID, AActor* Actor);

    /**
     * Returns all actors on the team specified by the TeamID. Returns an empty array if the TeamID is not found.
     */
    UFUNCTION()
    void GetActorsOnTeam(ETeam TeamID, TArray<AActor*>& OutActors);

    /**
     * Returns all Minions on the team specified by the TeamID. Returns an empty array if the TeamID is not found.
     */
    UFUNCTION()
    void GetMinionsOnTeam(ETeam TeamID, TArray<AAgoraMinionBase*>& OutMinions);

    /**
     * Returns all Heroes on the team specified by the TeamID. Returns an empty array if the TeamID is not found.
     */
    UFUNCTION()
    void GetHeroesOnTeam(ETeam TeamID, TArray<AAgoraHeroBase*>& OutHeroes);

    /**
     * Returns all players on a given team.
     */
    UFUNCTION()
    void GetPlayersOnTeam(ETeam TeamID, TArray<APlayerState*>& PlayerState);

	UFUNCTION()
	void GetViewTargetsOnTeam(ETeam TeamID, TArray<AActor*>& ViewTargets);

    UFUNCTION()
    void GenerateDebugString(FString& OutDebugString);

protected:

    // AInfo interface
    virtual void BeginPlay() override;
    // ~AInfo interface

    UFUNCTION()
    void SetupTeams();

    /** Array containing all teams in the game */
    UPROPERTY(Replicated)
    TArray<AAgoraTeam*> Teams;

    /** Getter to access teams, currently protected. Use public methods and specify a team id to access team internals */
    UFUNCTION()
    AAgoraTeam* GetTeamFromID(ETeam TeamID);

private:

    void PrintArray(TArray<AActor*>& ArrayToPrint, FString& OutString);

};
