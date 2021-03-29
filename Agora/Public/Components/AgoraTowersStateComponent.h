// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeamInterface.h"
#include "AgoraTypes.h"
#include "AgoraTowerBase.h"
#include "AgoraTowersStateComponent.generated.h"

class AAgoraTowerBase;
class AAgoraLaneSpawner;

USTRUCT(BlueprintType)
struct FLane
{
	GENERATED_BODY()

public:
	ELaneEnum Lane;

	UPROPERTY(EditDefaultsOnly)
	AAgoraTowerBase* T1Tower;

	UPROPERTY(EditDefaultsOnly)
	AAgoraTowerBase* T2Tower;

	UPROPERTY(EditDefaultsOnly)
	AAgoraTowerBase* Inhibitor;

	UPROPERTY(EditDefaultsOnly)
	AAgoraLaneSpawner* Spawner = nullptr;

	void SetTower(AAgoraTowerBase* Tower)
	{
		switch (Tower->GetTowerType())
		{
		case ETowerType::T1Tower:
			T1Tower = Tower;
			break;
		case ETowerType::T2Tower:
			T2Tower = Tower;
			break;
		case ETowerType::Inhibitor:
			Inhibitor = Tower;
			break;
		default:
			ensureMsgf(0, TEXT("Invalid tower passed to Lane SetTower"));
		}
	}

	AAgoraTowerBase* GetTower(ETowerType TowerType)
	{
		switch (TowerType)
		{
		case ETowerType::Invalid:
			ensureMsgf(0, TEXT("Get Tower requires a non invalid tower type"));
			return nullptr;
		case ETowerType::T1Tower:
			return T1Tower;
		case ETowerType::T2Tower:
			return T2Tower;
		case ETowerType::Inhibitor:
			return Inhibitor;
		case ETowerType::Core:
			// lanes do not have a core, teams do
			return nullptr;
		default:
			checkNoEntry();
			return nullptr;
		}
	}
};

USTRUCT(BlueprintType)
struct FTeamTowers {
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AAgoraTowerBase* TeamCore;

	FLane Lane0;
	FLane Lane1;
	FLane Lane2;

	FLane* GetLane(ELaneEnum LaneNum)
	{
		switch (LaneNum)
		{
		case ELaneEnum::LE_lane0:
			return &Lane0;
		case ELaneEnum::LE_lane1:
			return &Lane1;
		case ELaneEnum::LE_lane2:
			return &Lane2;
		default:
			ensureMsgf(0, TEXT("Failed to get lane! Returning nullptr"));
			return nullptr;
		}
	}
};

UCLASS( ClassGroup=(Agora), meta=(BlueprintSpawnableComponent) )
class AGORA_API UAgoraTowersStateComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void RespawnInhibitor(ETeam Team, ELaneEnum InhibLane);

public:
	UAgoraTowersStateComponent();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Towers status")
	FTeamTowers DuskTowers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Towers status")
	FTeamTowers DawnTowers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Towers status")
	float InhibitorRespawnRate = 480.0f;

	FTeamTowers* GetTeamTowers(ETeam Team);

	FLane* GetTeamLane(ETeam Team, ELaneEnum Lane);

	UFUNCTION(BlueprintCallable)
	AAgoraTowerBase* GetTower(ETeam Team, ELaneEnum Lane, ETowerType TowerType);

	void HandleTowerDeath(AAgoraTowerBase* TowerKilled);
};
