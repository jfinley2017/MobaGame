// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#pragma once

#include "CoreMinimal.h"
#include "AgoraCharacterBase.h"
#include "AgoraTowerBase.generated.h"

class USkeletalMesh;
class UGameplayEffect;
class USphereComponent;
class UAgoraTimeLevelComp;

UCLASS()
class AGORA_API AAgoraTowerBase : public AAgoraCharacterBase
{
	GENERATED_BODY()

public:
    AAgoraTowerBase(const FObjectInitializer& ObjectInitializer);

	void SetInvulnerable(bool bIsInvulnerable = true);
	FString ToString();
    ETowerType GetTowerType() const;
    ELaneEnum GetLane() const;

protected:

    // ACharacter interface
    virtual void BeginPlay() override;
    // ~ACharacter interface

    // AAgoraCharacter interface
	virtual void FinishPlayDying() override;
	virtual void StopPostDeath() override;
    // ~AAgoraCharacter interface

	UFUNCTION(BlueprintPure, Category = "Enemy List")
	bool ValidateEnemies();

	/**
	 * True if valid
	 */
	UFUNCTION(BlueprintPure, Category = "Enemy List")
	bool ValidateEnemy(AActor* Enemy);

    UFUNCTION()
    void NotifyTurretAreaOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
    UFUNCTION()
    void NotifyTurretAreaOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    /**Gets all enemies (minions and heroes) for the core splash damage*/
    UFUNCTION(BlueprintPure, Category = "Enemy List")
    TArray<AActor*> GetAllEnemies();

    //returns true if enemy minions are within the turrets area
    UFUNCTION(BlueprintCallable, Category = "Enemy List")
    bool CheckForEnemyMinion();

    /** 
	 * Checks for pawns to shoot at
	 * Return target if found
	*/
    UFUNCTION(BlueprintCallable, Category = "Enemy List")
    AActor* FindTarget();

    /**Set the Enemy hero that is shooting Friendly hero*/
    UFUNCTION(BlueprintCallable, Category = "Enemy List")
    void SetAggressiveEnemy(AActor* Enemy);

    /** Add hero to enemy hero list*/
    UFUNCTION(BlueprintCallable, Category = "Enemy List")
    void AddHero(AActor* Hero);

    /**Remove hero from enemy hero list*/
    UFUNCTION(BlueprintCallable, Category = "Enemy List")
    void RemoveHero(AActor* Hero);

    /**Add minion to enemy hero list*/
    UFUNCTION(BlueprintCallable, Category = "Enemy List")
    void AddEnemyMinion(class AActor* Minion);

    /**Remove minion from enemy hero list*/
    UFUNCTION(BlueprintCallable, Category = "Enemy List")
    void RemoveEnemyMinion(class AActor* Minion);

    void AddTrackedActor(AActor* const Target);
    void RemoveTrackedActor(AActor* const Target);
    void ResetIfTargetRemoved(AActor* const Target);
    void SetEnemyTarget(AActor* Target);

    //////////////////////////////////////////////////////////////////////////
    // Components 

    /** Sphere collision component */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turret Area")
    USphereComponent* TurretArea;

    /** Currently holding references to both meshes to swap between depending on the team. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Agora|AgoraTower|Initialization" )
    USkeletalMesh* DawnTeamMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Agora|AgoraTower|Initialization")
    USkeletalMesh* DuskTeamMesh;

    // ~Components
    //////////////////////////////////////////////////////////////////////////

    /** Cue to play when we are targeting an enemy. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|Tower")
    FGameplayTag TargetingCue;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|Tower")
    FGameplayTagContainer InvulnerableTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|Tower")
    TSubclassOf<UGameplayEffect> InvulnerableEffect;

    UPROPERTY(EditAnywhere, Category = "Agora|AgoraTower|Data")
    TSubclassOf<UGameplayEffect> DamageReductionRemove;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Agora|Tower")
    ETowerType Type;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Agora|Tower")
    ELaneEnum Lane;

    /** Used for checking how long turret needs to be in alert animation*/
    UPROPERTY(BlueprintReadWrite, Category = "Time", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastEnemy;

    UPROPERTY(BlueprintReadWrite, Category = Variable)
    float FireRate;

    /**Array of the five friendly heroes, needed to check if enemy is being aggressive.*/
    UPROPERTY(BlueprintReadWrite, Category = "Friendly List", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> FriendlyHeroes;

    /**Enemy has been targeted*/
    UPROPERTY(Replicated, BlueprintReadWrite, Category = Variable)
    bool bHasTargeted;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Enemy List")
    APawn* EnemyTarget;

    // Enemy heroes in the radius of the tower, distinguished from minions because heroes have different rules
    TArray<AActor*> EnemyHeroes;
    TArray<AActor*> EnemyMinions;

    FTimerHandle FinishFireHandle;

    bool bShouldTarget = true;

    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraTower")
    void ReceiveTurretAreaOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraTower")
    void ReceiveTurretAreaOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
