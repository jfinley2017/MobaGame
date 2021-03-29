// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Pawns/AgoraCharacterBase.h"
#include "AgoraInventoryComponent.h"
#include "AgoraHeroBase.generated.h"

DECLARE_DELEGATE_OneParam(FUpgradeAbilityDelegate, EAbilityInput);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputComponentReadySignature, UInputComponent*, InputComponent);

class UAgoraSkin;
class UAgoraXpLevelComponent;

/**
 * Base class for all heroes
 */
UCLASS()
class AGORA_API AAgoraHeroBase : public AAgoraCharacterBase
{
	GENERATED_BODY()
	

public:

    AAgoraHeroBase(const FObjectInitializer& ObjectInitializer);

    FInputComponentReadySignature OnInputComponentReady;

    // ACharacter interface
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual FVector GetPawnViewLocation() const override;
	virtual void PossessedBy(AController* NewController) override;
    // ~ACharacter interface

    // AgoraCharacterBase interface
	// Consider overriding notify level up
    virtual void SetTeam(ETeam NewTeam) override;
    virtual void NotifyTeamChanged(AActor* ChangedActor) override;
    virtual float CalculateDeathTimer() override;
    virtual bool GetRespawnTime(float& OutSpawnTimestamp) override;
    virtual void FinishPlayDying() override;
    // ~AgoraCharacterBase interface

    UFUNCTION(BlueprintPure, Category = "AgoraHero")
    UCameraComponent* GetCameraComponent();

	/**
	 * Server wrapper for AssignSkin
	 */
	UFUNCTION(Server, WithValidation, Reliable, Category = "AgoraHero|Skins")
	virtual void ServerAssignSkin();

    /**
     * Picks a skin, either using the first skin in hero metadata, or the selected skin in playerstate/game instance
     */
    UFUNCTION(Category = "AgoraHero|Skins")
    virtual void AssignSkin();

	/**
	 * Gets this hero's metadata, containing data about skins, hero name, etc.
	 */
    UFUNCTION(BlueprintPure, Category = "AgoraHero|Skins")
    virtual UAgoraHeroMetadata* GetHeroMetaData();

	USpringArmComponent* GetSpringArmComponent();
	
	// Made this public for ability testing, unsure if there's a 
	UFUNCTION(BlueprintCallable)
	void UpgradeAbility(EAbilityInput AbilityIndex);

protected:
	/**
	 * Actually handles setting up a particular skin, setting it on the mesh and such
	 */
	UFUNCTION(Category = "AgoraHero|Skins")
	void SetSkin(UAgoraSkin* Skin);

    // AAgoraCharacterBase interface
    virtual void NotifyPlayRespawnEffects() override;
    virtual void NotifyPlayDeathEffects(const FDamageContext& DamageContext) override;
    virtual void FinishDying() override;
    virtual void SetupRespawn(const FDamageContext& DamageContext) override;
    // ~AAgoraCharacterBase interface

    UFUNCTION()
    void MoveForward(float RelativeSpeed);
    UFUNCTION()
    void MoveRight(float RelativeSpeed);

    UFUNCTION(BlueprintCallable)
    bool CanUpgradeAbility(EAbilityInput InputId);
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerUpgradeAbility(EAbilityInput AbilityIndex);

    /** Finds the best focusable actor */
    AActor* FindFocusActor();

    /** Changed our focused actor to be @NewFocused */
    void SetFocusedActor(AActor* NewFocused);

    //////////////////////////////////////////////////////////////////////////
    // Components

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Agora|AgoraHero")
    UAgoraInventoryComponent* Inventory = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpringArmComponent")
    USpringArmComponent* SpringArmComponent = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraHero")
    UAgoraHeroMetadata* HeroMetadata = nullptr;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HeroSkin, Category = "Agora|AgoraHero")
    UAgoraSkin* HeroSkin = nullptr;

    // ~Components
    //////////////////////////////////////////////////////////////////////////

    AActor* CurrentlyFocusedActor;

    UFUNCTION()
    virtual void OnRep_HeroSkin();
};
