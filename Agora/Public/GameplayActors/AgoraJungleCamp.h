// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeamInterface.h"
#include "AgoraTypes.h"
#include "AgoraJungleCamp.generated.h"

class AAgoraJungleMinion;
class ATargetPoint;
class UTextRenderComponent;
class USphereComponent;

/**
 * Responsible for respawning jungle camps, displaying UI helpers, coordinating the jungle camp dynamics
 */
UCLASS()
class AGORA_API AAgoraJungleCamp : public AActor
{
	GENERATED_BODY()
	
public:	

	AAgoraJungleCamp();

	/**
	 * If any of the camp minions hard reset, they all should
	 */
	UFUNCTION()
	void NotifyStartResetCamp() const;

	UFUNCTION()
	void NotifyFinishResetCamp(AAgoraJungleMinion* FinishedMinion);

protected:

    //AActor
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    //~AActor

    UFUNCTION()
    void NotifyMatchStateChange(FName State);

    /**
     * Attempts to find a targetable actors within the camp area
     */
    UFUNCTION(BlueprintCallable)
    void GetTargetsInArea(TArray<AActor*>& OutTargets);

    UFUNCTION()
    void CampMinionDamageTaken(float CurrentHP, float MaxHP, const FDamageContext& Damage);

    UFUNCTION()
    void CampMinionDied(const FDamageContext& KillingBlow);

    void SpawnCamp();

    float GetInitialSpawnDelay();

    void StartRespawnTimer(float RespawnTime);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AgoraJungleCamp")
	UTextRenderComponent* CampText;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AgoraJungleCamp")
	USphereComponent* CampArea;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "AgoraJungleCamp")
	float LeashRadius = 1500.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="AgoraJungleCamp")
	float InitialSpawnDelay = 60.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "AgoraJungleCamp")
	float RespawnDelay = 60.0f;

	UPROPERTY(BlueprintReadWrite)
	bool bInCombat = false;

	UPROPERTY(BlueprintReadOnly)
	TArray<AAgoraJungleMinion*> CampMinions;

	UPROPERTY(EditInstanceOnly, Category = AgoraJungleCamp)
	TMap<ATargetPoint*, TSubclassOf<AAgoraJungleMinion>> SpawnPointMinionTypes;

	FTimerHandle RespawnTimer;

};
