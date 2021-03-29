// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TeamInterface.h"
#include "AgoraCharacterBase.h"
#include "Templates/SubclassOf.h"
#include "AbilitySystemInterface.h"
#include "AgoraPlayerState.generated.h"

class AAgoraCharacterBase;
class UAgoraStatTracker;
namespace ForwardDeclare { class UAgoraTeamComponent; }

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSkinChanged, UAgoraSkin*, Skin);


USTRUCT(BlueprintType)
struct AGORA_API FPlayerData
{
	GENERATED_BODY()
public:
	FUniqueNetIdRepl UniqueId;

    UPROPERTY(BlueprintReadOnly)
    ETeam Team;

	// TODO: Move this out of player data, and possibly out of player state
	UPROPERTY(BlueprintReadWrite)
	bool bIsReadyInLobby = false;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<AAgoraCharacterBase> HeroClass;
	
	UPROPERTY(BlueprintReadOnly)
	UAgoraSkin* HeroSkin = nullptr;

	FPlayerData() : UniqueId(FUniqueNetIdRepl()), Team(ETeam::Invalid), bIsReadyInLobby(false), HeroClass(nullptr) {}
	FPlayerData(FUniqueNetIdRepl Id) : UniqueId(Id), Team(ETeam::Invalid), bIsReadyInLobby(false), HeroClass(nullptr) {}
};

/**
 * Tracks information such as what should display in the scoreboard, controlled hero, etc.
 */
UCLASS()
class AGORA_API AAgoraPlayerState : public APlayerState, public ITeamInterface, public IAbilitySystemInterface,
    public IDamageReceiverInterface
{
	GENERATED_BODY()
	
public:

	AAgoraPlayerState(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(BlueprintAssignable, Category = "AgoraPlayerState")
    FOnTeamChangedSignature OnTeamChanged;
    UPROPERTY(BlueprintAssignable, Category = "AgoraPlayerState")
    FSkinChanged OnSkinChanged;
    UPROPERTY(BlueprintAssignable, Category = "AgoraPlayerState")
    FDamageReceivedSignature OnDamageReceived;
    UPROPERTY(BlueprintAssignable, Category = "AgoraPlayerState")
    FDiedSignature OnDied;

    // ITeamInterface
    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    ETeam GetTeam() const override;
    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    void SetTeam(ETeam NewTeam) override;
    virtual FOnTeamChangedSignature& GetTeamChangedDelegate() override;
    // ~TeamInterface

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    // ~IAbilitySystemInterface

    // IDamageReceiverInterface
    virtual void NotifyDamageReceived(float CurrentHP, float MaxHP, const FDamageContext& DamageContext) override;
    virtual void NotifyDied(const FDamageContext& DamageContext) override;
    virtual bool IsDead() const override;
    virtual FDamageReceivedSignature& GetDamageReceivedDelegate() override;
    virtual FDiedSignature& GetDeathDelegate() override;
    // ~IDamageReceiverInterface

    UFUNCTION()
    void NotifyTeamChanged(AActor* ChangedActor);

    UFUNCTION(BlueprintCallable, Category = "AgoraPlayerState")
    void SetPlayerReady(bool bIsReady = true);

    UFUNCTION(BlueprintCallable, Category = "AgoraPlayerState")
    void SetHeroClass(TSubclassOf<AAgoraCharacterBase> ClassToSet);

    UFUNCTION(BlueprintCallable, Category = "AgoraPlayerState")
    void SetSkin(UAgoraSkin* Skin);

    UFUNCTION(BlueprintPure, Category = "AgoraPlayerState")
    TSubclassOf<AAgoraCharacterBase> GetHeroClass();

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    virtual void BP_SetPlayerName(const FString& S);

	UFUNCTION(BlueprintPure)
	FPlayerData GetPlayerData() const;


   

protected:

    // APlayerState
	virtual void BeginPlay() override;
	virtual void ClientInitialize(class AController* C) override;
	virtual void CopyProperties(APlayerState* NewPlayerState) override;
	// ~APlayerState

    UFUNCTION(Server, WithValidation, Reliable)
    void ServerSetPlayerData(FPlayerData ClientLobbyData);

	// Makes sure that lobby data is replicated, and triggers on rep for client
	void SetPlayerData(FPlayerData DataToSet);

    // This is anything picked in drafting that has to be maintained across level travel
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerData, Category = "AgoraPlayerState")
    FPlayerData PlayerData = FPlayerData(UniqueId);

    UPROPERTY(VisibleAnywhere, Category = "AgoraPlayerState")
    UAgoraStatTracker* StatsTrackingComponent;

    UPROPERTY(VisibleAnywhere, Category = "AgoraPlayerState")
    UAgoraTeamComponent* TeamComponent;

    UFUNCTION()
	void OnRep_PlayerData(FPlayerData OldLobbyData);

};
