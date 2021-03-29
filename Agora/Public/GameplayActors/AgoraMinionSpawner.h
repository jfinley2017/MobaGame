#pragma once

#include "CoreMinimal.h"
#include "TeamInterface.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "AgoraMinionSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpawnMinionWave);

class UAgoraTimeLevelComp;

// Base class separate so that jungle spawning can be implemented separately from lane spawning later
UCLASS()
class AGORA_API AAgoraMinionSpawner : public AActor, public ITeamInterface
{
	GENERATED_BODY()
	
public:

    AAgoraMinionSpawner();

    UFUNCTION(BlueprintCallable)
    virtual void SpawnWave();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MinionSpawning")
    UAgoraTimeLevelComp* LevelComp = nullptr;

    UPROPERTY(BlueprintAssignable)
    FSpawnMinionWave OnSpawnMinionWave;

    // TeamInterface interface
    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    ETeam GetTeam() const override;
    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    void SetTeam(ETeam NewTeam) override;

    FOnTeamChangedSignature OnTeamChanged;
    virtual FOnTeamChangedSignature& GetTeamChangedDelegate() override;
    // ~TeamInterface interface
    
    UFUNCTION()
    virtual void NotifyLeveledUp(AActor* LeveledActor, int32 NewLevel, float LastLevelTime);

protected:

    virtual void BeginPlay() override;

	UFUNCTION()
	void HandleMatchStateChange(FName State);

    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraMinionSpawner")
    void ReceiveLeveledUp(AActor* LeveledActor, int32 NewLevel, float LastLevelTime);

    // Time in seconds for the first spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AgoraMinionSpawner")
	float FirstSpawnDelay = 60.f;

	// Delay between waves
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AgoraMinionSpawner")
    float DelayBetweenWaves = 30.f;

    // Timer that actually spawns a minion wave of 1 in each spawn point
    FTimerHandle MinionWaveTimerHandle;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "TeamInterface")
    ETeam Team = ETeam::Invalid;
};
