// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraJungleCamp.h"
#include <TimerManager.h>
#include "Engine/World.h"
#include "AgoraJungleMinion.h"
#include <Engine/TargetPoint.h>
#include <Map.h>
#include "Components/TextRenderComponent.h"
#include <Text.h>
#include <Internationalization.h>
#include "AgoraCharacterBase.h"
#include <Components/SphereComponent.h>
#include "Modes/AgoraGameState.h"
#include "Modes/AgoraGameMode.h"

AAgoraJungleCamp::AAgoraJungleCamp()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

	CampArea = CreateDefaultSubobject<USphereComponent>("CampArea");
	CampArea->SetSphereRadius(LeashRadius);
	CampArea->SetCollisionProfileName("OverlapOnlyPawn");

	RootComponent = CampArea;

	CampText = CreateDefaultSubobject<UTextRenderComponent>("CampText");
	CampText->WorldSize = 75.f;
	CampText->SetupAttachment(RootComponent);

	bOnlyRelevantToOwner = true;
	bNetLoadOnClient = false;
}

void AAgoraJungleCamp::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	// Make sure the camps have a potentially valid setup
	if (!ensure(SpawnPointMinionTypes.Num() > 0))
	{
		return;
	}

	GetWorld()->GetGameState<AAgoraGameState>()->OnMatchStateChanged.AddDynamic(this, &AAgoraJungleCamp::NotifyMatchStateChange);
}

void AAgoraJungleCamp::NotifyMatchStateChange(FName State)
{
	if (State == MatchState::GameBegun)
	{
		StartRespawnTimer(InitialSpawnDelay);
	}
}

void AAgoraJungleCamp::Tick(float DeltaSeconds)
{
	float TimeTillRespawn = GetWorld()->GetTimerManager().GetTimerRemaining(RespawnTimer);

	if (TimeTillRespawn > 0 && CampText)
	{
		FNumberFormattingOptions NumberFormat;
		NumberFormat.SetMaximumFractionalDigits(0);

		CampText->SetText(FText::Format(NSLOCTEXT("Agora", "RespawnText", "Respawning in {0}s"), FText::AsNumber(TimeTillRespawn, &NumberFormat)));
	}
}

void AAgoraJungleCamp::SpawnCamp()
{
    if (CampText)
    {
        CampText->SetHiddenInGame(true);
    }

	for (TPair<ATargetPoint*, TSubclassOf<AAgoraJungleMinion>> SpawnPointPair : SpawnPointMinionTypes)
	{
		TSubclassOf<AAgoraJungleMinion> ClassToSpawn = SpawnPointPair.Value;
		
		ESpawnActorCollisionHandlingMethod SpawnParams = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		AAgoraJungleMinion* Minion = GetWorld()->SpawnActorDeferred<AAgoraJungleMinion>(*ClassToSpawn, SpawnPointPair.Key->GetActorTransform(), nullptr, nullptr, SpawnParams);
		Minion->Camp = this;
		Minion->FinishSpawning(SpawnPointPair.Key->GetActorTransform());

		CampMinions.Add(Minion);
		
		ensure(CampMinions.Num() <= SpawnPointMinionTypes.Num());
		FScriptDelegate DamageTakenDelegate;
		DamageTakenDelegate.BindUFunction(this, "CampMinionDamageTaken");
		Minion->GetDamageReceivedDelegate().AddUnique(DamageTakenDelegate);
		//Minion->GetDamageReceivedDelegate().AddDynamic(this, &AAgoraJungleCamp::CampMinionDamageTaken);
		Minion->GetDeathDelegate().AddDynamic(this, &AAgoraJungleCamp::CampMinionDied);
	}
}

float AAgoraJungleCamp::GetInitialSpawnDelay()
{
	return InitialSpawnDelay;
}

void AAgoraJungleCamp::StartRespawnTimer(float RespawnTime)
{
	FTimerDelegate TimerCallback;
	if (GetNetMode() != NM_DedicatedServer)
	{
		CampText->SetHiddenInGame(false);
	}
	TimerCallback.BindUObject(this, &AAgoraJungleCamp::SpawnCamp);

	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, TimerCallback, RespawnTime, false);
}

void AAgoraJungleCamp::NotifyStartResetCamp() const
{
	for (AAgoraJungleMinion* Minion : CampMinions)
	{	
		if (Minion && !Minion->IsDead())
		{
			Minion->StartReset();
		}
	}
}

void AAgoraJungleCamp::NotifyFinishResetCamp(AAgoraJungleMinion* FinishedMinion)
{
	bool bAllMinionsFinished = true;
	for (AAgoraJungleMinion* Minion : CampMinions)
	{
		if (Minion && !Minion->IsDead() && Minion->bIsResetting)
		{
			// If any minions have not finished resetting, the camp is not yet finished resetting
			bAllMinionsFinished = false;
		}
	}

	// Enables the damage events triggering aggro again
	if (bAllMinionsFinished)
	{
		bInCombat = false;
	}

	for (AAgoraJungleMinion* Minion : CampMinions)
	{
		Minion->FullHeal();
	}
}

void AAgoraJungleCamp::GetTargetsInArea(TArray<AActor*>& OutTargets)
{
	CampArea->GetOverlappingActors(OutTargets, AAgoraCharacterBase::StaticClass());
	
	OutTargets = OutTargets.FilterByPredicate([](AActor* PotentialTarget) {
		ITeamInterface* TeamActor = Cast<ITeamInterface>(PotentialTarget);
		if (TeamActor)
		{
			ETeam Team = TeamActor->GetTeam();
			return Team == ETeam::Dawn || Team == ETeam::Dusk;
		}

		return false;
	});
}

void AAgoraJungleCamp::CampMinionDamageTaken(float CurrentHP, float MaxHP, const FDamageContext& Damage)
{
	if (bInCombat)
	{
		return;
	}

	bInCombat = true;

	for (AAgoraJungleMinion* Minion : CampMinions)
	{
		Minion->SetTarget(Damage.DamagingEffectContext.GetOriginalInstigator());
	}
}

void AAgoraJungleCamp::CampMinionDied(const FDamageContext& KillingBlow)
{
	AAgoraJungleMinion* DeadMinion = Cast<AAgoraJungleMinion>(KillingBlow.DamagedActor);
	
	// Not sure what we do if the minion is already null'd, let's hope for now that won't happen
	if (ensure(DeadMinion))
	{
		DeadMinion->GetDeathDelegate().RemoveDynamic(this, &AAgoraJungleCamp::CampMinionDied);
		DeadMinion->GetDamageReceivedDelegate().RemoveDynamic(this, &AAgoraJungleCamp::CampMinionDamageTaken);
		CampMinions.Remove(DeadMinion);
	}

	// Is the camp dead?
	if (CampMinions.Num() == 0)
	{
		// Reset the camp
		bInCombat = false;
		StartRespawnTimer(RespawnDelay);
	}
}
