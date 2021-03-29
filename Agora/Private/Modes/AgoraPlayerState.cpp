// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#include "AgoraPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "AgoraLobbyState.h"
#include "AgoraGameState.h"
#include "Data/AgoraSkin.h"
#include "Data/AgoraHeroMetadata.h"
#include "AgoraStatTracker.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AgoraTeamComponent.h"
#include "AgoraHeroBase.h"

AAgoraPlayerState::AAgoraPlayerState(const FObjectInitializer& ObjectInitializer)
{
    StatsTrackingComponent = CreateDefaultSubobject<UAgoraStatTracker>(TEXT("StatsComponent"));
    TeamComponent = CreateDefaultSubobject<UAgoraTeamComponent>(TEXT("TeamComponent"));
}

void AAgoraPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AAgoraPlayerState, PlayerData);
}


void AAgoraPlayerState::BeginPlay()
{
    Super::BeginPlay();

    TeamComponent->OnTeamChanged.AddDynamic(this, &AAgoraPlayerState::NotifyTeamChanged);

    

}

// If this gets unwieldy, can split into individual handler functions for each property in the struct, or rethink how to do this
void AAgoraPlayerState::OnRep_PlayerData(FPlayerData OldPlayerData)
{

	if (OldPlayerData.HeroSkin != PlayerData.HeroSkin)
	{
		OnSkinChanged.Broadcast(PlayerData.HeroSkin);
	}

    AAgoraLobbyState* LobbyState = GetWorld()->GetGameState<AAgoraLobbyState>();

	if (!LobbyState) // are we in lobby or not
	{
		return; 
	}

	if (OldPlayerData.HeroClass != PlayerData.HeroClass)
	{
		LobbyState->HeroClassChangedDel.Broadcast(PlayerData.HeroClass, this);
	}

	if (OldPlayerData.bIsReadyInLobby != PlayerData.bIsReadyInLobby)
	{
		LobbyState->PlayerReadyDel.Broadcast(PlayerData.bIsReadyInLobby, this);
	}
}

UAbilitySystemComponent* AAgoraPlayerState::GetAbilitySystemComponent() const
{
    return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn());
}

void AAgoraPlayerState::SetPlayerReady(bool bIsReady /*= true*/)
{
	if (PlayerData.HeroClass && GetTeam() != ETeam::Invalid) 
    {
		FPlayerData NewPD = PlayerData;
		NewPD.bIsReadyInLobby = bIsReady;
		SetPlayerData(NewPD);
	}
}

// This is really intended for use in the lobby, currently won't do anything in game
void AAgoraPlayerState::SetHeroClass(TSubclassOf<AAgoraCharacterBase> ClassToSet)
{
	FPlayerData NewPD = PlayerData;
	NewPD.HeroClass = ClassToSet;
	SetPlayerData(NewPD);
}

void AAgoraPlayerState::SetSkin(UAgoraSkin* Skin)
{
	FPlayerData NewPD = PlayerData;
	NewPD.HeroSkin = Skin;
	SetPlayerData(NewPD);
}

void AAgoraPlayerState::BP_SetPlayerName(const FString& S)
{
	SetPlayerName(S);
}

void AAgoraPlayerState::NotifyTeamChanged(AActor* ChangedActor)
{
    ITeamInterface* PawnAsTeamInterface = Cast<ITeamInterface>(GetPawn());
    if (PawnAsTeamInterface && PawnAsTeamInterface->GetTeam() != GetTeam())
    {
        PawnAsTeamInterface->SetTeam(GetTeam());
    }


    AAgoraLobbyState* LobbyState = GetWorld()->GetGameState<AAgoraLobbyState>();
    if (LobbyState)
    {
        LobbyState->TeamChangedDel.Broadcast(GetTeam(), this);
    }
    
}

FPlayerData AAgoraPlayerState::GetPlayerData() const
{
	return PlayerData;
}

void AAgoraPlayerState::NotifyDamageReceived(float CurrentHP, float MaxHP, const FDamageContext& DamageContext)
{
    OnDamageReceived.Broadcast(CurrentHP, MaxHP, DamageContext);
}

void AAgoraPlayerState::NotifyDied(const FDamageContext& DamageContext)
{
    OnDied.Broadcast(DamageContext);
}

bool AAgoraPlayerState::IsDead() const
{
    return UAgoraEntityLibrary::IsDead(GetPawn());
}

FDamageReceivedSignature& AAgoraPlayerState::GetDamageReceivedDelegate()
{
    return OnDamageReceived;
}

FDiedSignature& AAgoraPlayerState::GetDeathDelegate()
{
    return OnDied;
}

void AAgoraPlayerState::ClientInitialize(class AController* C)
{
	Super::ClientInitialize(C);

	if (!C->GetCharacter())
	{
		return;
	}

	AAgoraHeroBase* Hero = Cast<AAgoraHeroBase>(C->GetCharacter());

	if (!Hero)
	{
		return;
	}

	Hero->ServerAssignSkin();
}

void AAgoraPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

    TRACE(Agora, Log, "Playerstate handoff: %s -> %s", *GetNameSafe(this), *GetNameSafe(NewPlayerState));

	AAgoraPlayerState* NewPlayerStateAsAgoraPlayerState = Cast<AAgoraPlayerState>(NewPlayerState);

	if (NewPlayerStateAsAgoraPlayerState)
	{
		NewPlayerStateAsAgoraPlayerState->SetPlayerData(PlayerData);
	}
}

void AAgoraPlayerState::SetPlayerData(FPlayerData DataToSet)
{
	FPlayerData OldPD = PlayerData;
	DataToSet.UniqueId = UniqueId; // makes sure this is always up to date
	PlayerData = DataToSet;
    TeamComponent->SetTeam(PlayerData.Team);
	// OnRep even if we're the client so that it's updated immediately -- we still want replication updates that come from the server though 
	OnRep_PlayerData(OldPD); // provide old PD to avoid unnecessary delegates firing

	if (!HasAuthority())
	{
		ServerSetPlayerData(PlayerData);
	}
}

void AAgoraPlayerState::ServerSetPlayerData_Implementation(FPlayerData ClientLobbyData)
{
	SetPlayerData(ClientLobbyData);
}

bool AAgoraPlayerState::ServerSetPlayerData_Validate(FPlayerData ClientLobbyData)
{
	return true;
}

TSubclassOf<AAgoraCharacterBase> AAgoraPlayerState::GetHeroClass()
{
	return PlayerData.HeroClass;
}

ETeam AAgoraPlayerState::GetTeam() const
{
    return PlayerData.Team;
}

void AAgoraPlayerState::SetTeam(ETeam NewTeam)
{
    FPlayerData NewPD = PlayerData;
    NewPD.Team = NewTeam;
    SetPlayerData(NewPD);
}

FOnTeamChangedSignature& AAgoraPlayerState::GetTeamChangedDelegate()
{
    return TeamComponent->OnTeamChanged;
}

