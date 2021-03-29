// Not For Public Use or Distribution

#include "AgoraTowersStateComponent.h"
#include "Agora.h"
#include "AgoraTowerBase.h"
#include "AgoraLaneSpawner.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "AgoraAttributeSetBase.h"
#include "AgoraWorldSettings.h"
#include "AgoraTypes.h"

UAgoraTowersStateComponent::UAgoraTowersStateComponent()
{
}

void UAgoraTowersStateComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> TowersFound;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAgoraTowerBase::StaticClass(), TowersFound);

	// auto add the lanes to the right team structs
	for (AActor* Actor : TowersFound) {

		AAgoraTowerBase* Tower = Cast<AAgoraTowerBase>(Actor);

		AAgoraWorldSettings* WorldSettings = Cast<AAgoraWorldSettings>(GetWorld()->GetWorldSettings());
		if (Tower->GetTowerType() != WorldSettings->LastTower)
		{
			Tower->SetInvulnerable();
		}

		ensureMsgf(Tower->GetTowerType() != ETowerType::Invalid, TEXT("%s has invalid tower type"), *Tower->GetName());

		if (Tower->GetTowerType() == ETowerType::Core)
		{
			GetTeamTowers(Tower->GetTeam())->TeamCore = Tower;
		}
		else
		{
			GetTeamLane(Tower->GetTeam(), Tower->GetLane())->SetTower(Tower);
		}
		
	}

	TArray<AActor*> SpawnersFound;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAgoraLaneSpawner::StaticClass(), SpawnersFound);

	for (AActor* SpawnerActor : SpawnersFound)
	{
		AAgoraLaneSpawner* LaneSpawner = Cast<AAgoraLaneSpawner>(SpawnerActor);

		ETeam SpawnerTeam = LaneSpawner->GetTeam();
		ELaneEnum SpawnerLane = LaneSpawner->GetLane();

		GetTeamLane(SpawnerTeam, SpawnerLane)->Spawner = LaneSpawner;
	}
}

void UAgoraTowersStateComponent::RespawnInhibitor(ETeam Team, ELaneEnum InhibLane)
{
	AAgoraTowerBase* Tower = GetTower(Team, InhibLane, ETowerType::Inhibitor);

	if (Tower)
	{
		Tower->SetActorHiddenInGame(false);
	}
}

FTeamTowers* UAgoraTowersStateComponent::GetTeamTowers(ETeam Team)
{
	switch (Team)
	{
		case ETeam::Dawn:
			return &DawnTowers;
		case ETeam::Dusk:
			return &DuskTowers;
		default:
			ensureMsgf(0, TEXT("GetTeamTowers must be called with a valid team option"));
			return nullptr;
	}
}

FLane* UAgoraTowersStateComponent::GetTeamLane(ETeam Team, ELaneEnum Lane)
{
	return GetTeamTowers(Team)->GetLane(Lane);
}

AAgoraTowerBase* UAgoraTowersStateComponent::GetTower(ETeam Team, ELaneEnum Lane, ETowerType TowerType)
{
	FTeamTowers* TeamTowers = GetTeamTowers(Team);
	
	if (TowerType == ETowerType::Core)
	{
		return TeamTowers->TeamCore;
	}

	return GetTeamLane(Team, Lane)->GetTower(TowerType);
}

void UAgoraTowersStateComponent::HandleTowerDeath(AAgoraTowerBase* TowerKilled)
{
	if (TowerKilled->GetTowerType() == ETowerType::Core || TowerKilled->GetTowerType() == ETowerType::Invalid)
	{
		return;
	}

	if (TowerKilled->GetTowerType() == ETowerType::Inhibitor)
	{
		ETeam TeamSpawningSupers = TowerKilled->GetTeam() == ETeam::Dawn ? ETeam::Dusk : ETeam::Dawn;
		FLane* TowerLane = GetTeamLane(TeamSpawningSupers, TowerKilled->GetLane());

		TowerLane->Spawner->SetIsSpawningSuper(true);

		FTimerHandle InhibRespawnHandle;
		FTimerDelegate TimerDel;
		ETeam Team = TowerKilled->GetTeam();
		ELaneEnum Lane = TowerKilled->GetLane();

		TimerDel.BindLambda([&, Team, Lane]()
		{
			RespawnInhibitor(Team, Lane);
		});

		GetWorld()->GetTimerManager().SetTimer(InhibRespawnHandle, TimerDel, InhibitorRespawnRate, false);
	}

	ETowerType NextTowerType = ETowerType::Invalid;

	// Could just do some casting to uint8, increment, but could lead to confusing bugs if enum were changed
	switch (TowerKilled->GetTowerType())
	{
	case ETowerType::T1Tower:
		NextTowerType = ETowerType::T2Tower;
		break;
	case ETowerType::T2Tower:
		NextTowerType = ETowerType::Inhibitor;
		break;
	case ETowerType::Inhibitor:
		NextTowerType = ETowerType::Core;
		break;
	default:
		checkNoEntry();
		break;
	}

	AAgoraTowerBase* NextTower = GetTower(TowerKilled->GetTeam(), TowerKilled->GetLane(), NextTowerType);
	
	TRACE(AgoraAI, Log, "Tower team %d lane %d type %d", (uint8)TowerKilled->GetTeam(), (uint8)TowerKilled->GetLane(), (uint8)NextTowerType);

	if (NextTower && NextTower->IsValidLowLevel())
	{
		TRACE(AgoraAI, Log, "Setting tower %s vulnerable", *NextTower->ToString());
		NextTower->SetInvulnerable(false);
	}
	// Could check here to see if all 3 inhibitors are dead and do something if so

}
