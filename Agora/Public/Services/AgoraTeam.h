// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Interfaces/TeamInterface.h"
#include "AgoraTeam.generated.h"

class AAgoraHeroBase;
class AAgoraMinionBase;
class APlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeroJoinedTeamSignature, ETeam, Team, AAgoraHeroBase*, Hero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeroLeftTeamSignature, ETeam, Team, AAgoraHeroBase*, Hero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerJoinedTeamSignature, ETeam, Team, APlayerState*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeftTeamSignature, ETeam, Team, APlayerState*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMinionJoinedTeamSignature, ETeam, Team, AAgoraMinionBase*, Minion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMinionLeftTeamSignature, ETeam, Team, AAgoraMinionBase*, Minion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiscActorJoinedTeamSignature, ETeam, Team, AActor*, MiscActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiscActorLeftTeamSignature, ETeam, Team, AActor*, MiscActor);

// #TODO Joe 
// These arrays need to be converted into FFastSerializer arrays.

/**
 * Object representing team data.
 */
UCLASS()
class AGORA_API AAgoraTeam : public AInfo
{
	GENERATED_BODY()
	
public:

    AAgoraTeam();

    UPROPERTY(BlueprintAssignable)
    FOnHeroJoinedTeamSignature OnHeroJoinedTeam;
    UPROPERTY(BlueprintAssignable)
    FOnHeroLeftTeamSignature OnHeroLeftTeam;
    UPROPERTY(BlueprintAssignable)
    FOnMinionJoinedTeamSignature OnMinionJoinedTeam;
    UPROPERTY(BlueprintAssignable)
    FOnMinionLeftTeamSignature OnMinionLeftTeam;
    UPROPERTY(BlueprintAssignable)
    FOnPlayerJoinedTeamSignature OnPlayerJoinedTeam;
    UPROPERTY(BlueprintAssignable)
    FOnPlayerLeftTeamSignature OnPlayerLeftTeam;
    UPROPERTY(BlueprintAssignable)
    FOnMiscActorJoinedTeamSignature OnMiscActorJoinedTeam;
    UPROPERTY(BlueprintAssignable)
    FOnMiscActorLeftTeamSignature OnMiscActorLeftTeam;

    UFUNCTION()
    void NotifyHeroJoinedTeam(AAgoraHeroBase* Hero);
    UFUNCTION()
    void NotifyHeroLeftTeam(AAgoraHeroBase* Hero);

    UFUNCTION()
    void NotifyMinionJoinedTeam(AAgoraMinionBase* Minion);
    UFUNCTION()
    void NotifyMinionLeftTeam(AAgoraMinionBase* Minion);

    UFUNCTION()
    void NotifyPlayerJoinedTeam(APlayerState* Player);
    UFUNCTION()
    void NotifyPlayerLeftTeam(APlayerState* Player);

    UFUNCTION()
    void NotifyMiscActorJoinedTeam(AActor* Actor);
    UFUNCTION()
    void NotifyMiscActorLeftTeam(AActor* Actor);

    UFUNCTION()
    void GetAllActorsOnTeam(TArray<AActor*>& OutActors);
    UFUNCTION()
    void GetHeroesOnTeam(TArray<AAgoraHeroBase*>& OutHeroes);
    UFUNCTION()
    void GetPlayersOnTeam(TArray<APlayerState*>& OutPlayers);
    UFUNCTION()
    void GetMinionsOnTeam(TArray<AAgoraMinionBase*>& OutMinions);
	UFUNCTION()
	void GetViewTargetsOnTeam(TArray<AActor*>& OutViewTargets);

    UFUNCTION()
    ETeam GetTeamID();
    UFUNCTION()
    void SetTeamID(ETeam NewTeamID);

protected:

    // AInfo
    virtual void BeginPlay();
    // ~AInfo

    UFUNCTION(Client, Reliable)
    void ClientNotifyPlayerJoinedTeam(APlayerState* Player);
    UFUNCTION(Client, Reliable)
    void ClientNotifyPlayerLeftTeam(APlayerState* Player);

    UPROPERTY(Replicated)
    ETeam TeamID = ETeam::Invalid;
    
    UPROPERTY(Replicated)
    TArray<AAgoraHeroBase*> Heroes;

    UPROPERTY(Replicated)
    TArray<AAgoraMinionBase*> Minions;

    UPROPERTY(Replicated)
    TArray<AActor*> MiscActors;

    UPROPERTY(Replicated)
    TArray<APlayerState*> Players;

	UPROPERTY(Replicated)
	TArray<AActor*> ViewTargets;

};
