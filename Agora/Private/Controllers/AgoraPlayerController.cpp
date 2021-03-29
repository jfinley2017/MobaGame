#include "AgoraPlayerController.h"
#include "AgoraCharacterBase.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "Lib/AgoraCheatManager.h"
#include "AgoraGameState.h"
#include "AgoraPlayerState.h"
#include "AgoraUserWidget.h"
#include "AgoraWaitForPlayersWidget.h"
#include "Lib/Options/AgoraUserSettings.h"
#include "Modes/AgoraGameMode.h"
#include "WidgetBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraActor.h"

DEFINE_LOG_CATEGORY(AgoraPlayerController);

AAgoraPlayerController::AAgoraPlayerController()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	AgoraCheatManager = CreateDefaultSubobject<UAgoraCheatManager>(TEXT("AgoraCheatManager"));
#endif

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f;

}

/**
 * Patch in support for console commands on our cheat manager
 * This is only required because we don't extend the native cheat manager, which is a UObject,
 * and thus doesn't support RPC's, which we need in order for this thing to be convenient
 *
 * The way exec is handled is vaguely like a visitor pattern, after a command is entered, it goes down
 * the tree of things that can handle exec commands. We're just extending that tree by registering our 
 * cheat manager
 */
bool AAgoraPlayerController::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
	bool handled = Super::ProcessConsoleExec(Cmd, Ar, Executor);
	if (!handled)
	{
		handled = AgoraCheatManager->ProcessConsoleExec(Cmd, Ar, Executor);
	}
	return handled;
}

void AAgoraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UAgoraUserSettings::Get()->ApplyMouseSensitivity(this);

	bool bIsSpectator = (GetAgoraPawn() == nullptr);

	
	if (GetNetMode() != NM_DedicatedServer && IsLocalPlayerController() && !bIsSpectator)
	{
		for (TSubclassOf<UUserWidget> WidgetClass : WidgetsToRemoveOnBegin)
		{
			TArray<UUserWidget*> FoundWidgets;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, WidgetClass);

			for (UUserWidget* Widget : FoundWidgets)
			{
				Widget->RemoveFromParent();
			}
		}

        // Spawn the HUD
        SetupHUD();

		NotifyServerClientLoaded();
	}

	SetupPawn(GetPawn());
}

void AAgoraPlayerController::NotifyServerClientLoaded_Implementation()
{
	AAgoraGameMode* AGM = GetWorld()->GetAuthGameMode<AAgoraGameMode>();

	if (AGM)
	{
		AGM->NotifyClientLoaded();
	}
}

bool AAgoraPlayerController::NotifyServerClientLoaded_Validate()
{
	return true;
}

void AAgoraPlayerController::SetSpectating(bool bShouldSpectate /*= true*/)
{
	if (bShouldSpectate) 
    {
		ChangeState(NAME_Playing);
	}
	else 
    {
		ChangeState(NAME_Spectating);
	}
}

void AAgoraPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	ITeamInterface* PossessedUnitAsTeamInterface = Cast<ITeamInterface>(NewPawn);
	if (PossessedUnitAsTeamInterface)
	{
		AAgoraPlayerState* PS = GetPlayerState<AAgoraPlayerState>();
		if (PS)
		{
			FPlayerData PD = PS->GetPlayerData();
			PossessedUnitAsTeamInterface->SetTeam(PD.Team != ETeam::Invalid ? PD.Team : ETeam::Dusk);
		}
	}
    
	Client_OnPossess(NewPawn);
}

void AAgoraPlayerController::OnUnPossess()
{
	APawn* Old = GetPawn();
    Client_UnPossess(Old);
    Super::OnUnPossess();
}

void AAgoraPlayerController::Client_OnPossess_Implementation(APawn* NewPawn)
{
    if (NewPawn)
	{
        SetupPawn(NewPawn);
    }
}

void AAgoraPlayerController::Client_UnPossess_Implementation(APawn* OlPawn)
{
    if (OlPawn)
    {
        UnsetupPawn(OlPawn);
    }
}

void AAgoraPlayerController::SetupPawn(APawn* NewPawn)
{
	if (!NewPawn)
	{
		return;
	}

	IDamageReceiverInterface* NewPawnAsDamageReceiver = Cast<IDamageReceiverInterface>(NewPawn);
    IDamageDealerInterface* NewPawnAsDamageDealer = Cast<IDamageDealerInterface>(NewPawn);

    if (NewPawnAsDamageDealer)
    {
        FScriptDelegate DamageInstigatedDelegate;
        DamageInstigatedDelegate.BindUFunction(this, FName("NotifyCausedHit"));
        NewPawnAsDamageDealer->GetDamageInstigatedDelegate().AddUnique(DamageInstigatedDelegate);
    }

    if (NewPawnAsDamageReceiver)
    {
        FScriptDelegate DamageReceivedDelegate;
        DamageReceivedDelegate.BindUFunction(this, FName("NotifyTakeHit"));
        NewPawnAsDamageReceiver->GetDamageReceivedDelegate().AddUnique(DamageReceivedDelegate);

        FScriptDelegate DiedDelegate;
        DiedDelegate.BindUFunction(this, "NotifyDied");
        NewPawnAsDamageReceiver->GetDeathDelegate().AddUnique(DiedDelegate);

    }

	FScriptDelegate PawnRespawnDelegate;
	PawnRespawnDelegate.BindUFunction(this, FName("NotifyPawnRespawned"));
    if (Cast<AAgoraCharacterBase>(NewPawn))
    {
        Cast<AAgoraCharacterBase>(NewPawn)->OnRespawned.AddUnique(PawnRespawnDelegate);
    }
}

void AAgoraPlayerController::SetupHUD()
{
    // Assuming there is only one player controller per client here.
    static int32 NumFailedSetupFrames = 0;

    AAgoraGameState* AgoraGameState = GetWorld()->GetGameState<AAgoraGameState>();
    AAgoraPlayerState* AgoraPlayerState = GetPlayerState<AAgoraPlayerState>();
    if (!AgoraGameState || !AgoraPlayerState)
    {
        NumFailedSetupFrames++;
        TRACE(AgoraPlayerController, Log, "Failed to find AgoraGameState and/or AgoraPlayerState. Delaying HUD setup until next frame. %d failures.", NumFailedSetupFrames)
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AAgoraPlayerController::SetupHUD);
        return;
    }
    
    TRACE(AgoraPlayerController, Log, "Initializing HUD after %d failures.", NumFailedSetupFrames)
    HUDWidget = CreateWidget<UAgoraUserWidget>(this, HUDWidgetClass);
    if (HUDWidget)
    {
        HUDWidget->AddToViewport();
    }
   
}

void AAgoraPlayerController::UnsetupPawn(APawn* PawnToUnset)
{
    if (PawnToUnset)
    {
        IDamageReceiverInterface* PawnToUnsetAsDamageReceiver = Cast<IDamageReceiverInterface>(PawnToUnset);
        IDamageDealerInterface* PawnToUnsetAsDamageDealer = Cast<IDamageDealerInterface>(PawnToUnset);

        if (PawnToUnsetAsDamageDealer)
        {
            FScriptDelegate DamageInstigatedDelegate;
            DamageInstigatedDelegate.BindUFunction(this, FName("NotifyCausedHit"));
            PawnToUnsetAsDamageDealer->GetDamageInstigatedDelegate().AddUnique(DamageInstigatedDelegate);
        }

        if (PawnToUnsetAsDamageReceiver)
        {
            FScriptDelegate DamageReceivedDelegate;
            DamageReceivedDelegate.BindUFunction(this, FName("NotifyTakeHit"));
            PawnToUnsetAsDamageReceiver->GetDamageReceivedDelegate().AddUnique(DamageReceivedDelegate);
        }

        FScriptDelegate PawnRespawnDelegate;
        PawnRespawnDelegate.BindUFunction(this, FName("NotifyPawnRespawned"));
        if (Cast<AAgoraCharacterBase>(PawnToUnset))
        {
            Cast<AAgoraCharacterBase>(PawnToUnset)->OnRespawned.AddUnique(PawnRespawnDelegate);
        }
    }
}

void AAgoraPlayerController::NotifyTakeHit(float CurrentHealth, float MaxHealth, const FDamageContext& HitDamage)
{
   
}

void AAgoraPlayerController::NotifyCausedHit(const FDamageContext& HitDamage)
{
   
}

void AAgoraPlayerController::NotifyMatchFinished(APawn* DestroyedCore)
{

    ReceiveMatchFinished(DestroyedCore);
}

AAgoraCharacterBase* AAgoraPlayerController::GetAgoraPawn() const
{
	return Cast<AAgoraCharacterBase>(GetPawn());
}

void AAgoraPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bPostDeathPan)
    {
        if (!PostDeathViewTarget.Get())
        {
            UnsetupPostDeathPan();
        }

        FVector DeathViewTargetLocation = PostDeathViewTarget->GetActorLocation();
        FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
        FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, DeathViewTargetLocation);
       
        PostDeathPanCamera->SetActorRotation(LookAtRotation);
        
    }

}

void AAgoraPlayerController::NotifyCoreDestroyed(AActor* Core, const FDamageContext KillingBlow)
{
   
    ReceiveCoreDestroyed(Core, KillingBlow);
}

void AAgoraPlayerController::NotifyPawnRespawned()
{
    SetViewTargetWithBlend(GetPawn());
}

void AAgoraPlayerController::NotifyHeroKilled(AActor* Hero, FDamageContext KillingBlow)
{
    ReceiveHeroKilled(Hero, KillingBlow);



}

void AAgoraPlayerController::NotifyTowerDestroyed(AActor* Tower, const FDamageContext KillingBlow)
{
    ReceiveTowerDestroyed(Tower, KillingBlow);
}

ETeam AAgoraPlayerController::GetTeam() const
{
    return GetAgoraPawn() ? GetAgoraPawn()->GetTeam() : ETeam::Invalid;
}

void AAgoraPlayerController::SetTeam(ETeam NewTeam)
{
	if (GetAgoraPawn())
	{
		GetAgoraPawn()->SetTeam(NewTeam);
	}
}

FOnTeamChangedSignature& AAgoraPlayerController::GetTeamChangedDelegate()
{
	if (GetAgoraPawn())
	{
		return Cast<ITeamInterface>(GetAgoraPawn())->GetTeamChangedDelegate();
	}
	else
	{
        return OnTeamChanged;
	}
}


void AAgoraPlayerController::NotifyDied(const FDamageContext& KillingBlow)
{
    ReceivePawnDied(KillingBlow);


    SetupPostDeathPan(KillingBlow.DamageInstigator);


}

void AAgoraPlayerController::SetupPostDeathPan(AActor* Killer)
{
    if (!GetPawn() || !Killer)
    {
        TRACE(AgoraPlayerController, Warning, "Could not play death camera pan. Pawn did not exist");
        SetupPostDeathSpectate();
        return;
    }

    PostDeathViewTarget = Killer;

    FVector DeathLocation = GetPawn()->GetActorLocation();
    FVector DeathCameraSpawnLocation = FVector(DeathLocation.X, DeathLocation.Y, DeathLocation.Z + 250.0f);

    if (!PostDeathPanCamera)
    {
        PostDeathPanCamera = GetWorld()->SpawnActor<ACameraActor>();
    }

    PostDeathPanCamera->SetActorLocation(DeathCameraSpawnLocation);
    PostDeathPanCamera->SetActorRotation(FRotator(0, 0, 0));

    SetViewTarget(PostDeathPanCamera);

    // #TODO Joe make this based on respawn time
    float RespawnTime = 5.0f;

    /*UAgoraEntityLibrary::GetRemainingRespawnDuration(GetPawn(), RespawnTime);
    RespawnTime = RespawnTime < 10.0f ? RespawnTime / 3.0f : 5.0f;*/

    GetWorld()->GetTimerManager().SetTimer(PostDeathPanCameraExpireTimerHandle, this, &AAgoraPlayerController::UnsetupPostDeathPan, RespawnTime, false, RespawnTime);

    bPostDeathPan = true;

}

void AAgoraPlayerController::UnsetupPostDeathPan()
{
    bPostDeathPan = false;
    PostDeathViewTarget = nullptr;

    SetupPostDeathSpectate();


}

void AAgoraPlayerController::SetupPostDeathSpectate()
{
    TArray<AAgoraPlayerState*> Players;
    AAgoraGameState* GS = Cast<AAgoraGameState>(GetWorld()->GetGameState());

    if (GS)
    {
        GS->GetTeamPlayerStates(GetTeam(), Players);

        for (AAgoraPlayerState* APS : Players)
        {
            AAgoraCharacterBase* AChar = Cast<AAgoraCharacterBase>(APS->GetPawn());

            if (AChar && !AChar->IsDead())
            {
                SetViewTargetWithBlend(AChar, 0.2f);
            }
            else
            {
                UE_LOG(Agora, Log, TEXT("Could not find teammate to spectate"));
            }
        }
    }
}

