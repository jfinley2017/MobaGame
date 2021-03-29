// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#include "AgoraGameMode.h"
#include "GameFramework/Controller.h"
#include "AgoraTowersStateComponent.h"
#include "Modes/AgoraPlayerState.h"
#include "Modes/AgoraGameState.h"
#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#endif
#include "Kismet/GameplayStatics.h"
#include "AgoraPlayerStart.h"
#include "AgoraPlayerController.h"
#include "AgoraMenuController.h"
#include "AgoraGameplayTags.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraTowerBase.h"
#include "TimerManager.h"
#include "Lib/AgoraWorldSettings.h"
#include "Lib/AgoraBlueprintFunctionLibrary.h"

namespace MatchState
{
	const FName BeginGameCountdown = FName(TEXT("BeginGameCountdown"));
	const FName GameBegun = FName(TEXT("GameBegun"));
}

AAgoraGameMode::AAgoraGameMode()
{
	//Gamelift Server-side code
#if WITH_GAMELIFT

	//Getting the module first.
	FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

	//InitSDK establishes a local connection with GameLift's agent to enable communication.
	GameLiftSdkModule->InitSDK();

	//Respond to new game session activation request. GameLift sends activation request 
	//to the game server along with a game session object containing game properties 
	//and other settings. Once the game server is ready to receive player connections, 
	//invoke GameLiftServerAPI.ActivateGameSession()
	auto OnGameSession = [=](Aws::GameLift::Server::Model::GameSession GameSession)
	{
		GameLiftSdkModule->ActivateGameSession();
	};

	FProcessParameters* Params = new FProcessParameters();
	Params->OnStartGameSession.BindLambda(OnGameSession);

	//OnProcessTerminate callback. GameLift invokes this before shutting down the instance 
	//that is hosting this game server to give it time to gracefully shut down on its own. 
	//In this example, we simply tell GameLift we are indeed going to shut down.
	Params->OnTerminate.BindLambda([=]() {GameLiftSdkModule->ProcessEnding(); });

	//HealthCheck callback. GameLift invokes this callback about every 60 seconds. By default, 
	//GameLift API automatically responds 'true'. A game can optionally perform checks on 
	//dependencies and such and report status based on this info. If no response is received  
	//within 60 seconds, health status is recorded as 'false'. 
	//In this example, we're always healthy!
	Params->OnHealthCheck.BindLambda([]() {return true; });

	//Here, the game server tells GameLift what port it is listening on for incoming player 
	//connections. In this example, the port is hardcoded for simplicity. Since active game
	//that are on the same instance must have unique ports, you may want to assign port values
	//from a range, such as:
	//const int32 port = FURL::UrlConfig.DefaultPort;
	//params->port;
	Params->port = 7777;

	//Here, the game server tells GameLift what set of files to upload when the game session 
	//ends. GameLift uploads everything specified here for the developers to fetch later.
	TArray<FString> Logfiles;
	Logfiles.Add("/../../logs/AgoraServerLog.txt");
	Params->logParameters = Logfiles;

	//Call ProcessReady to tell GameLift this game server is ready to receive game sessions!
	GameLiftSdkModule->ProcessReady(*Params);
#endif
	TowersState = CreateDefaultSubobject<UAgoraTowersStateComponent>(TEXT("TowersState"));
	PlayerControllerClass = AAgoraMenuController::StaticClass();
}

FString AAgoraGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal /*= TEXT("")*/)
{
	//in here, we handle incoming player connections.
	//Gamelift requires us to validate incoming connections, i.e. if a player is matched by the matchmaker,
	//the matchmaker creates a player slot for them, but the player who is connecting must supply
	//the playerID and the PlayerSessionID that they received when they found a match.
	//otherwise gamelift will not let us connect.
	//so we do it here.

#if WITH_GAMELIFT

	FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

	FString PlayerId = UGameplayStatics::ParseOption(Options, TEXT("PlayerId")); //we'd pair this with the corresponding entry in matchmaking data to do whatever, it's not needed right now
	
	FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId")); //this one's the big cheese, though
	GameLiftSdkModule->AcceptPlayerSession(PlayerSessionId);
#endif
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

// Overridden to handle PIE
void AAgoraGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	HandleConnectPlayer();
}

void AAgoraGameMode::NotifyKilled(AActor* Killer, AActor* Killed, FDamageContext DamageContext)
{
    IGameplayTagAssetInterface* KilledAsTagAsset = Cast<IGameplayTagAssetInterface>(Killed);
    AAgoraGameState* GS = GetGameState<AAgoraGameState>();

    if (KilledAsTagAsset && GS)
    {
        if (KilledAsTagAsset->HasMatchingGameplayTag(UAgoraGameplayTags::Hero()))
        {
            GS->NetMulticast_HeroKilled(Killed, DamageContext);
        }
        if (KilledAsTagAsset->HasMatchingGameplayTag(UAgoraGameplayTags::Tower()) && !KilledAsTagAsset->HasMatchingGameplayTag(UAgoraGameplayTags::Core()))
        {
			TowersState->HandleTowerDeath(Cast<AAgoraTowerBase>(Killed));
            GS->NetMulticast_TowerDestroyed(Killed, DamageContext);
        }
        if (KilledAsTagAsset->HasMatchingGameplayTag(UAgoraGameplayTags::Core()))
        {
			//Important for later:
			/*
			I just now added the call to FinishMatch(). Previously that function wasn't being used anywhere despite it being integral to finishing off the gameplay cycle.
			Not sure why.
			In FinishMatch(), we're just calling into GameState's OnMatchComplete where we're both disabling everyone's input and handling other server side stuff.
			However, we're also disabling input inside CoreDestroyed(), which currently just calls into BP stubs.
			Unless I'm missing something, CoreDestroyed and FinishMatch/OnMatchComplete are effectively the same state in the game's overall state machine.
			The match ends when one of the cores is destroyed.
			Therefore there should be a cleaner way to do this. 
			Is there a need for both of these functions?
			- paca
			*/
            GS->NetMulticast_CoreDestroyed(Killed, DamageContext);
			FinishMatch(Cast<APawn>(Killed));
        }
    }
}

void AAgoraGameMode::FinishMatch(APawn* DestroyedCore)
{
	AAgoraGameState* GS = GetGameState<AAgoraGameState>();
	if (GS)
	{
		GS->NetMulticast_OnMatchComplete(DestroyedCore);
	}
}

AAgoraGameState* AAgoraGameMode::GetAgoraGameState()
{
	AAgoraGameState* AGS = GetGameState<AAgoraGameState>();
	ensure(AGS);

	return AGS;
}

// Called from the end of client's AgoraPlayerController BeginPlay
void AAgoraGameMode::NotifyClientLoaded()
{
	HandleConnectPlayer();
}

bool AAgoraGameMode::AllClientsConnected()
{
	int32 TotalPlayers = GameState->PlayerArray.Num();
	return NumClientsLoaded >= TotalPlayers;
}

void AAgoraGameMode::HandleConnectPlayer()
{
	// In progress is actually waiting to begin countdown, we use it just because of the base game mode code involved
	// The alternative is to copy paste a bunch of gamemodebase code, or not use the "GameMode" (rather GameModeBase) and come up with our own match state thing


	FName CurrentMatchState = GetAgoraGameState()->GetMatchState();
	
	bool bIsWaitingToStart = CurrentMatchState == MatchState::InProgress || CurrentMatchState == MatchState::WaitingToStart;
	if (!bIsWaitingToStart)
	{
		// Only handle connecting players at the beginning of the game, PostLogin/HandleSeamlessTravelPlayer gets called at the end of the game too, for some reason
		return;
	}

	// This is used rather than "traveling players" because traveling players changes before the client is fully loaded
	NumClientsLoaded++;

	if (!GetAgoraGameState())
	{
		TRACE(Agora, Warning, "HandleConnectPlayer requires AgoraGameState");
		return;
	}

	int32 TotalPlayers = GameState->PlayerArray.Num();
	GetAgoraGameState()->NotifyPlayersChanged(TotalPlayers - NumClientsLoaded, NumClientsLoaded);

	if (AllClientsConnected())
	{
		FTimerHandle GameCountdownTimer;

		HandleBeginCountdown();

		float CountTime = UAgoraBlueprintFunctionLibrary::GetAgoraWorldSettings(this)->GetGameBeginCountdownDuration();

		GetWorld()->GetTimerManager().SetTimer(GameCountdownTimer, this, &AAgoraGameMode::HandleBeginGame, CountTime, false);
	}
}

void AAgoraGameMode::HandleBeginCountdown()
{
	GetAgoraGameState()->SetMatchState(MatchState::BeginGameCountdown);
}

void AAgoraGameMode::HandleBeginGame()
{
	GetAgoraGameState()->SetMatchState(MatchState::GameBegun);
}

// change which hero class is spawned in as a result of draft picks
UClass* AAgoraGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (!InController)
	{
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	AAgoraPlayerState* APS = InController->GetPlayerState<AAgoraPlayerState>();

	if (!APS)
	{
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	FPlayerData LobbyData = APS->GetPlayerData();

	if (LobbyData.HeroClass)
	{
		return LobbyData.HeroClass;
	}

	// if we don't have a class, just use the pawn class set in world settings
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AAgoraGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> StartsFound;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAgoraPlayerStart::StaticClass(), StartsFound);

	// Will have to handle AI differently here or just cast to team interface
	AAgoraPlayerController* PC = Cast<AAgoraPlayerController>(Player);
	if (!PC)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	UClass* PawnClass = GetDefaultPawnClassForController(Player);
	APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr; // ripped from GameModeBase

	AAgoraPlayerState* APS = PC->GetPlayerState<AAgoraPlayerState>();

	if (APS)
	{
		FPlayerData LobbyData = APS->GetPlayerData();

		// GS hasn't had a chance to initialize yet at this point and set the PS with the correct team
		ETeam PlayerTeam = LobbyData.Team;

		// without dedicated server, this is called before player state begin play and therefore this isn't set yet
		if (PlayerTeam == ETeam::Invalid)
		{
			PlayerTeam = ETeam::Dusk;
		}

		for (auto PlayerStartActor : StartsFound)
		{
			AAgoraPlayerStart* PlayerStart = Cast<AAgoraPlayerStart>(PlayerStartActor);

			if (PlayerTeam == PlayerStart->GetTeam() && 
				!GetWorld()->EncroachingBlockingGeometry(PawnToFit, PlayerStartActor->GetActorLocation(), PlayerStartActor->GetActorRotation()))
			{
				return PlayerStart;
			}
		}
		
		UE_LOG(AgoraMode, Warning, TEXT("No AgoraPlayerStarts for team %d were found"), (uint8)PlayerTeam);
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

bool AAgoraGameMode::CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget)
{
	return UAgoraBlueprintFunctionLibrary::IsFriendly(Viewer, ViewTarget);
}
