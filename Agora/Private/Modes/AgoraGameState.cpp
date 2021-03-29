// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#include "AgoraGameState.h"
#include "Engine/World.h"
#include "Agora.h"
#include "Controllers/AgoraPlayerController.h"
#include "AgoraPlayerState.h"
#include "AgoraGameInstance.h"
#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#endif
#include "AgoraBlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundCue.h"
#include "AgoraWorldSettings.h"
#include "AgoraVisionService.h"
#include "AgoraTeamService.h"
#include "AgoraMapTrackerService.h"
#include "AgoraItemService.h"
#include "Modes/AgoraGameMode.h"
#include "AgoraGameLiftService.h"


void AAgoraGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAgoraGameState, TeamService);
    DOREPLIFETIME(AAgoraGameState, ItemService);
    DOREPLIFETIME(AAgoraGameState, VisionService);
	DOREPLIFETIME(AAgoraGameState, NumConnectedPlayers);
	DOREPLIFETIME(AAgoraGameState, NumTravellingPlayers);
	DOREPLIFETIME(AAgoraGameState, TimeMatchBegan);
	DOREPLIFETIME(AAgoraGameState, TimeCountdownBegan);
}

void AAgoraGameState::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    if (HasAuthority())
    {
        TeamService = GetWorld()->SpawnActor<AAgoraTeamService>();
        ItemService = GetWorld()->SpawnActor<AAgoraItemService>();
        VisionService = GetWorld()->SpawnActor<AAgoraVisionService>();
        OnRep_TeamService();
        OnRep_ItemService();
        OnRep_VisionService();
    }
   
    if (GetNetMode() != NM_DedicatedServer)
    {
        MapTrackerService = GetWorld()->SpawnActor<AAgoraMapTrackerService>();
    }
}

void AAgoraGameState::HandlePlayersChanged()
{
    OnNumPlayersChanged.Broadcast();
}

void AAgoraGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	TimeMatchBegan = ReplicatedWorldTimeSeconds;
}

float AAgoraGameState::GetSecondsSinceMatchStart()
{
	return GetServerWorldTimeSeconds() - TimeMatchBegan;
}

int32 AAgoraGameState::GetNumConnectedPlayers()
{
	return NumConnectedPlayers;
}

int32 AAgoraGameState::GetNumMaxPlayers()
{
	return NumConnectedPlayers + NumTravellingPlayers;
}

void AAgoraGameState::NotifyPlayersChanged_Implementation(int32 InNumTravellingPlayers, int32 InNumConnectedPlayers)
{
	NumTravellingPlayers = InNumTravellingPlayers;
	NumConnectedPlayers = InNumConnectedPlayers;

	HandlePlayersChanged();
}

bool AAgoraGameState::NotifyPlayersChanged_Validate(int32 InNumTravellingPlayers, int32 InNumConnectedPlayers)
{
	return true;
}

void AAgoraGameState::NetMulticast_OnMatchComplete_Implementation(APawn* DestroyedCore)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AAgoraPlayerController* PC = Cast<AAgoraPlayerController>(It->Get());
		if (PC && PC->IsLocalController())
		{
			PC->NotifyMatchFinished(DestroyedCore);
			
			// Disable Input
			APawn* MyPawn = PC->GetPawn();
			if (MyPawn)
			{
				MyPawn->DisableInput(PC);
			}
		}
	}
	
	//post game stuff (server only)
	if (HasAuthority())
	{
		TRACE(Agora, Log, "Begin post match server stuff");
		//game's ended, so send every client back to the main menu (or wherever)
		//follow with a call to Disconnect() to kick everyone from the server, and then shut it down.
		GetWorld()->ServerTravel(PostMatchLevelName);
		NetMulticast_Disconnect();
		
#if WITH_GAMELIFT
		TRACE(Agora, Log, "Terminating GameLift game session");
		FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));
		GameLiftSdkModule->InitSDK();
		GameLiftSdkModule->TerminateGameSession();
#endif
	}
}

void AAgoraGameState::NetMulticast_HeroKilled_Implementation(AActor* Hero, FDamageContext KillingBlow)
{
    OnHeroKilled.Broadcast(Hero, KillingBlow);
	USoundCue* PlayerDeathSound = UAgoraBlueprintFunctionLibrary::GetWorldSettings(Hero)->PlayerDeathSound;
	UGameplayStatics::PlaySoundAtLocation(Hero, PlayerDeathSound, FVector::ZeroVector);
}

void AAgoraGameState::NetMulticast_TowerDestroyed_Implementation(AActor* Tower, FDamageContext KillingBlow)
{
    OnTowerKilled.Broadcast(Tower, KillingBlow);
	USoundCue* TowerDeathSound = UAgoraBlueprintFunctionLibrary::GetWorldSettings(Tower)->TowerDeathSound;
	UGameplayStatics::PlaySoundAtLocation(Tower, TowerDeathSound, FVector::ZeroVector);
}


void AAgoraGameState::NetMulticast_CoreDestroyed_Implementation(AActor* Core, FDamageContext KillingBlow)
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AAgoraPlayerController* PC = Cast<AAgoraPlayerController>(It->Get());
        if (PC)
        {
            PC->NotifyCoreDestroyed(Core, KillingBlow);
        }
    }
}

void AAgoraGameState::GetAgoraPlayers(TArray<AAgoraPlayerState*>& Arr)
{
	Arr.Empty();

	for (APlayerState* PS : PlayerArray)
	{
		AAgoraPlayerState* APS = Cast<AAgoraPlayerState>(PS);
		Arr.Add(APS);
	}
}

void AAgoraGameState::GetTeamPlayerStates(ETeam Team, TArray<AAgoraPlayerState*>& OutArr)
{
	TArray<AAgoraPlayerState*> Players;
	GetAgoraPlayers(Players);

	for (AAgoraPlayerState* APS : Players)
	{
		if (APS->GetTeam() == Team)
		{
			OutArr.Add(APS);
		}
	}
}

AAgoraVisionService* AAgoraGameState::GetVisionService() const
{
    return VisionService;
}

AAgoraMapTrackerService* AAgoraGameState::GetMapTrackerService()
{
    if (GetNetMode() == NM_DedicatedServer) { return nullptr; }

    if (MapTrackerService)
    {
        return MapTrackerService;
    }

    MapTrackerService = GetWorld()->SpawnActor<AAgoraMapTrackerService>();
    return MapTrackerService;
}

AAgoraTeamService* AAgoraGameState::GetTeamService() const
{
    return TeamService;
}

AAgoraItemService* AAgoraGameState::GetItemService() const
{
    return ItemService;
}

float AAgoraGameState::GetTimeSinceCountdownStarted()
{
	return GetServerWorldTimeSeconds() - TimeCountdownBegan;
}

void AAgoraGameState::NetMulticast_Disconnect_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		TRACE(Agora, Log, "Tried to call disconnect on the server but it should only execute on clients, returning");
		return;
	}
	TRACE(Agora, Log, "Calling disconnect client");
	UGameplayStatics::GetPlayerController(this, 0)->ConsoleCommand("disconnect");

	//since we're sending this player back to the main menu (or post game screen, more precisely), delete the temporary file containing the game session info
	UAgoraBlueprintFunctionLibrary::DeleteTextFile(GameLiftFileName);
}

void AAgoraGameState::OnRep_MatchState()
{
	Super::OnRep_MatchState();

	// Clients predict the countdown
	if (MatchState == MatchState::BeginGameCountdown && TimeCountdownBegan == 0)
	{
		TimeCountdownBegan = GetServerWorldTimeSeconds();
	}

	// Clock time is not predicted
	if (MatchState == MatchState::GameBegun && HasAuthority())
	{
		TimeMatchBegan = GetServerWorldTimeSeconds();
	}

	OnMatchStateChanged.Broadcast(MatchState);
}

void AAgoraGameState::OnRep_NumConnectedPlayers()
{
	HandlePlayersChanged();
}

void AAgoraGameState::OnRep_NumTravellingPlayers()
{
	HandlePlayersChanged();
}

void AAgoraGameState::OnRep_TeamService()
{
    OnTeamServiceReady.Broadcast(TeamService);
}

void AAgoraGameState::OnRep_ItemService()
{
    OnItemServiceReady.Broadcast(ItemService);
}

void AAgoraGameState::OnRep_VisionService()
{
    OnVisionServiceReady.Broadcast(VisionService);
}
