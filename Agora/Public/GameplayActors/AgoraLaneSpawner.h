#pragma once

#include "CoreMinimal.h"
#include "GameplayActors/AgoraMinionSpawner.h"
#include "AgoraTimeLevelComp.h"
#include "AgoraTypes.h"
#include "AgoraLevelComponent.h"
#include "AgoraLaneSpawner.generated.h"

class AAgoraLaneMinion;
class AAgoraLaneWaypoint;

UCLASS()
class AGORA_API AAgoraLaneSpawner : public AAgoraMinionSpawner
{
	GENERATED_BODY()


public:
	virtual void SpawnWave() override;

	void SpawnWaveMinion();

	// Setter to manage state changes
	UFUNCTION()
	void SetIsSpawningSuper(bool isSuper);

	UFUNCTION()
	bool GetIsSpawningSuper();

    UFUNCTION()
    ELaneEnum GetLane() const;

    UFUNCTION()
    void RefreshCurrentSpawnLevel();

    // AAgoraMinionSpawner interface
    virtual void NotifyLeveledUp(AActor* LeveledActor, int32 NewLevel, float LastLevelTime) override;
    // ~AAgoraMinionSpawner interface

protected:
    virtual void BeginPlay() override;

	// Time in seconds to separate spawning minions each wave
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AgoraMinionSpawning", meta = (AllowPrivateAccess = "true"))
	float TimeBetweenMinions = 1.f;

    UPROPERTY(EditInstanceOnly, Category = "AgoraMinionSpawning")
    ELaneEnum Lane = ELaneEnum::LE_lane0;

    UPROPERTY(EditInstanceOnly, Category = "AgoraMinionSpawning") 
    TArray<AAgoraLaneWaypoint*> Waypoints;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AgoraMinionSpawning", meta = (AllowPrivateAccess = "true"))
    bool bIsSpawningSuper = false;

    // This is a row of the level table
    UPROPERTY(BlueprintReadOnly)
    FLaneSpawnLevel CurrentSpawnLevel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraMinionSpawning", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AAgoraLaneMinion> MeleeMinionClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraMinionSpawning", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AAgoraLaneMinion> RangedMinionClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraMinionSpawning", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AAgoraLaneMinion> SiegeMinionClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraMinionSpawning", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AAgoraLaneMinion> SuperMinionClass = nullptr;


    // I'd rather pass these into the timer function somehow but I'm not sure how to do it
    // The number left to be spawned in this current wave
	// EDIT: I know how to do it now, but I can't be bothered atm

    uint8 MeleeMinionsToSpawn = 0;
    uint8 RangedMinionsToSpawn = 0;
    uint8 SiegeMinionsToSpawn = 0;
	uint8 SuperMinionsToSpawn = 0;

    int32 WaveCount = 0;
    FTimerHandle MinionWaveSpawningTimerHandle;
};
