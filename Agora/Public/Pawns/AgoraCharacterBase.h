// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "GameplayTagAssetInterface.h"
#include "TeamInterface.h"
#include "AbilitySystemInterface.h"
#include "DamageDealerInterface.h"
#include "DamageReceiverInterface.h"
#include "FocusableInterface.h"
#include "VisionInterface.h"
#include "MinimapDisplayableInterface.h"

#include "AttributeSets/AgoraAttributeSetBase.h" // Default TSubclassOf
#include "AttributeSets/AgoraAttributeSetHealth.h" // Default TSubclassOf
#include "AttributeSets/AgoraAttributeSetMovement.h" // Default TSubclassOf
#include "AttributeSets/AgoraAttributeSetStatsBase.h" // Default TSubclassOf
#include "AttributeSets/AgoraAttributeSetEconomyBase.h" // Defaults TSubclassOf

#include "AgoraTypes.h" // FDamageContext, EAbilityInput

#include "AgoraCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventDelegate);
DECLARE_DELEGATE_OneParam(FUpgradeAbilityDelegate, EAbilityInput);

class UCameraComponent;
class USpringArmComponent;
class UAgoraDeathRewardComponent;
class UAgoraSceneTargetComponent;
class UAgoraAbilitySystemComponent;
class UAgoraXpLevelComponent;
class UGASCharacterMovementComponent;
class USphereComponent;
class UCameraShake;
class UAnimInstance;
class UAnimMontage;
class UCurveFloat;
class UTimelineComponent;
class UAgoraMaterialEffectsComponent;
class UAgoraHeroMetadata;
class UAgoraSkin;
class UAgoraVisionComponent;
class UAgoraTeamComponent;

UCLASS()
class AGORA_API AAgoraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ITeamInterface, public IGameplayTagAssetInterface, public IDamageDealerInterface, public IDamageReceiverInterface,
    public IFocusableInterface, public IVisionInterface, public IMinimapDisplayableInterface
{
	GENERATED_BODY()

public:

    AAgoraCharacterBase(const FObjectInitializer& ObjectInitializer);

    FOnHiddenLevelChangedSignature HiddenLevelChanged;
    FOnFocusedActorSignature OnStartFocused;
    FOnFocusedActorSignature OnEndFocused;
    FOnFocusedActorSignature OnFocusChanged;
    UPROPERTY(BlueprintAssignable, Category = "Agora|Character|Events")
    FDamageReceivedSignature OnDamageReceived;
    UPROPERTY(BlueprintAssignable, Category = "Agora|Character|Events")
    FDamageInstigatedSignature OnDamageInstigated;
    UPROPERTY(BlueprintAssignable, Category = "Agora|Character|Events")
    FEventDelegate OnRespawned;
    UPROPERTY(BlueprintAssignable, Category = "AgoraCharacter")
    FDiedSignature OnDeath;
    
    //////////////////////////////////////////////////////////////////////////
    // Interfaces

    // ACharacter interface
	virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void PreInitializeComponents() override;
    // ~ACharacter interface

    // IAbilitySystemComponent interface
    UFUNCTION(BlueprintCallable, Category = "AgoraUnitInterface")
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    // ~IAbilitySystemComponent interface

    // IGameplayTagAssetInterface interface
    UFUNCTION(BlueprintCallable, Category = GameplayTags)
    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
    UFUNCTION(BlueprintCallable, Category = GameplayTags)
    virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
    UFUNCTION(BlueprintCallable, Category = GameplayTags)
    virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
    UFUNCTION(BlueprintCallable, Category = GameplayTags)
    virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
    // ~IGameplayTagAssetInterface interface

    // IDamageDealerInterface interface
    UFUNCTION()
    virtual void NotifyDamageInstigated(const FDamageContext& DamageContext);

    virtual FDamageInstigatedSignature& GetDamageInstigatedDelegate() override;
    // ~IDamageDealerInterface interface

    // IDamageReceiverInterface interface
    UFUNCTION()
    virtual void NotifyDamageReceived(float CurrentHP, float MaxHP, const FDamageContext& DamageContext);
    UFUNCTION()
    void NotifyDied(const FDamageContext& DamageContext);
    UFUNCTION(BlueprintPure, Category = "AgoraCharacter")
    virtual bool IsDead() const;

    virtual FDamageReceivedSignature& GetDamageReceivedDelegate() override;
    virtual FDiedSignature& GetDeathDelegate() override;
    // ~IDamageReceiverInterface interface

    // ITeamInterface interface
    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    ETeam GetTeam() const override;
    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    void SetTeam(ETeam NewTeam) override;

    UFUNCTION()
    virtual void NotifyTeamChanged(AActor* ChangedActor);

    virtual FOnTeamChangedSignature& GetTeamChangedDelegate() override;
    // ~ITeamInterface interface

    // IFocusable interface
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Focusable")
    void NotifyFocused(AActor* FocusingActor);
    virtual void NotifyFocused_Implementation(AActor* FocusingActor) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Focusable")
    void NotifyEndFocused(AActor* EndFocusingActor);
    virtual void NotifyEndFocused_Implementation(AActor* EndFocusingActor) override;

    virtual FOnFocusedActorSignature& GetStartFocusedDelegate() override;
    virtual FOnFocusedActorSignature& GetEndFocusedDelegate() override;
    virtual FOnFocusedActorSignature& GetFocusChangedDelegate() override;
    // ~IFocusable interface

    // IVision interface
    virtual void NotifyRevealed(AActor* Revealer) override;
    virtual void NotifyRevealedEnd(AActor* StoppedRevealer) override;
    virtual bool IsVisibleFor(AActor* ViewingActor) const override;
    virtual void GetViewLocation(FVector& OutViewLocation) const override;
    virtual uint8 GetVisionLevel() const override;
    virtual uint8 GetHiddenLevel() const override;
    virtual FOnVisibilityToggledSignature& GetBecameVisibleDelegate() override;
    virtual FOnVisibilityToggledSignature& GetBecameHiddenDelegate() override;
    virtual FOnVisionLevelChangedSignature& GetVisionLevelChangedDelegate() override;
    virtual FOnHiddenLevelChangedSignature& GetHiddenLevelChangedDelegate() override;
    // ~IVision interface

    // IMinimapDisplayableInterface
    virtual UAgoraMapIconComponent* GetMapIconComponent() override;
    // ~IMinimapDisplayable interface

    // ~Interfaces
    //////////////////////////////////////////////////////////////////////////

    /** Notifys this character that it leveled up */
    UFUNCTION()
    virtual void NotifyLevelUp(AActor* LeveledActor, int32 NewLevel, float LevelupTimestamp);

    UFUNCTION(BlueprintCallable, Category = "AgoraUnitInterface")
    UAgoraAbilitySystemComponent* GetAgoraAbilitySystemComponent() const;

    /** Retrieves this character's max speed */
    UFUNCTION(BlueprintCallable, Category = "AgoraCharacter")
    float GetMaxSpeed();

    /** 
     * Gets the timestamp (in seconds) at which this character will respawn. Returns false if the target isn't dead or the time could otherwise not be specified
     */
    UFUNCTION(BlueprintPure, Category = "AgoraCharacter")
    virtual bool GetRespawnTime(float& OutSpawnTimestamp);

    /** Retreives this character's level */
    UFUNCTION(BlueprintPure, Category = "Level")
    uint8 GetLevel();

	UFUNCTION(BlueprintPure, Category = "Level")
	UAgoraLevelComponent* GetLevelComponent();
    
    UFUNCTION(BlueprintPure, Category = "AgoraCharacter")
    virtual float CalculateBasicAttackTime();

    UFUNCTION(BlueprintPure, Category = "AgoraCharacter")
    virtual UAgoraSceneTargetComponent* GetTargetComponent();
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomNode = 0) override;
    UGASCharacterMovementComponent* GetGASCMC();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Character Icon for the Minimap
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    UTexture* CharacterMinimapIcon;


protected:

    // ACharacter interface
    virtual void BeginPlay() override;
    // ~ACharacter interface

    // Initializes attribute sets based on TSubclassOf values specified
    void SetUpAttributeSets();

    /**
     * Returns how long this character should be dead for after dying
     */
    virtual float CalculateDeathTimer();

    /**
     * Validates the character. Usually used to set all critical values in order, like enabling input etc
     */
    UFUNCTION()
    virtual void ValidateCharacter();

    /**
     * Hook to allow subclasses to stop themselves after dying. Called prior to death effects are played. 
     */
    UFUNCTION()
    virtual void StopPostDeath();

    /** 
     * Stops this character from receiving any more commands, AI or otherwise. 
     */
    UFUNCTION()
    virtual void DisableAllInput();

    /**
     * Removes collision for this actor entirely
     */
    UFUNCTION()
    virtual void DisableCollision();

    /**
     * Called by the server after giving time to play death effects.
     */
    UFUNCTION()
    virtual void FinishDying();

	/** Called after the death anim finishes */
	UFUNCTION()
	virtual void FinishPlayDying();

    /** Plays effects related to taking damage */
    UFUNCTION()
    virtual void NotifyPlayDamagedEffects(float CurrentHP, float MaxHP, const FDamageContext& DamageContext);

    /** Plays effects related to dying */
    UFUNCTION()
    virtual void NotifyPlayDeathEffects(const FDamageContext& DamageContext);

    UFUNCTION()
    virtual void SetupRespawn(const FDamageContext& DamageContext);

    /** [Server Only] */
    UFUNCTION()
    virtual void NotifyRespawned(FGameplayTag Tag, int32 Count);

    UFUNCTION()
    virtual void NotifyPlayRespawnEffects();

    /** [All] Play animations, set visible, etc */
    UFUNCTION(NetMulticast, Reliable, Category = "AgoraCharacterEvents")
    virtual void NetMulticast_CharacterRespawned();

    /** [All] Play animations, set hidden locally, etc */
    UFUNCTION(NetMulticast, Reliable, Category = "AgoraCharacterEvents")
    virtual void NetMulticast_Death(FDamageContext DamageContext);
    void NetMulticast_Death_Implementation(FDamageContext DamageContext);

    //////////////////////////////////////////////////////////////////////////
    // Components 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AgoraCharacter")
	UAgoraDeathRewardComponent* DeathRewardComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AgoraCharacter")
	UAgoraMaterialEffectsComponent* MaterialEffects = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AgoraCharacter")
    UAgoraAbilitySystemComponent* AbilitySystemComponent = nullptr;

    // Provides delegates for handling level up events as well as the general leveling logic
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "AgoraCharacter")
    UAgoraXpLevelComponent* LevelComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AgoraCharacter")
    UAgoraVisionComponent* VisionComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AgoraCharacter")
    UAgoraTeamComponent* TeamComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AgoraCharacter")
    UAgoraMapIconComponent* MapIconComponent = nullptr;

    // The spot on this unit where targeted elements should be drawn/go towards
    // Could potentially be used to draw damage numbers/etc, however we might need
    // a separate component for that
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AgoraCharacter")
    UAgoraSceneTargetComponent* TargetLocationComponent = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Agora|AgoraCharacter")
    TSubclassOf<UAgoraAttributeSetBase> BaseAttributeSetClass = UAgoraAttributeSetBase::StaticClass();

    UPROPERTY(EditDefaultsOnly, Category = "Agora|AgoraCharacter")
    TSubclassOf<UAgoraAttributeSetHealth> HealthAttributeSetClass = UAgoraAttributeSetHealth::StaticClass();

    UPROPERTY(EditDefaultsOnly, Category = "Agora|AgoraCharacter")
    TSubclassOf<UAgoraAttributeSetMovement> MovementAttributeSetClass = UAgoraAttributeSetMovement::StaticClass();

    UPROPERTY(EditDefaultsOnly, Category = "Agora|AgoraCharacter")
    TSubclassOf<UAgoraAttributeSetStatsBase> StatsAttributeSetClass = UAgoraAttributeSetStatsBase::StaticClass();

    UPROPERTY(EditDefaultsOnly, Category = "Agora|AgoraCharacter")
    TSubclassOf<UAgoraAttributeSetEconomyBase> EconomyAttributeSetClass = UAgoraAttributeSetEconomyBase::StaticClass();

    // Optional additional attribute sets that one wishes to grant this character
    UPROPERTY(EditDefaultsOnly, Category = "Agora|AgoraCharacter")
    TArray<TSubclassOf<UAgoraAttributeSetBase>> AdditionalAttributeSetClasses;

    // ~Components
    //////////////////////////////////////////////////////////////////////////

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    FGameplayTagContainer DefaultGameplayTags;

   
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    UAnimMontage* DeathAnim;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    UAnimMontage* RespawnAnim;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    UAnimMontage* HitReactAnim;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    USoundBase* HitReactSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    UAnimMontage* RecallMontage;

    /** 
     * The cue which is fired when we successfully respawn 
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    FGameplayTag RespawnGameplayCue;

    /** 
     * The Cue which is fired when this unit dies. Should handle hiding the actor (for now) 
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    FGameplayTag DeathGameplayCue;

    /**
     * The Cue which is fired when this unit instigates damage.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    FGameplayTag InstigatedDamageCue;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora|AgoraCharacter")
    FGameplayTag LevelupGameplayCue;
 
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceiveEndFocused(AActor* EndFocusingActor);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceiveFocused(AActor* FocusingActor);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceivePlayDamagedEffects(float CurrentHP, float MaxHP, const FDamageContext& DamageContext);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceivePlayDeathEffects(const FDamageContext& DamageContext);
	UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
	void ReceiveFinishPlayDying();
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceiveRespawned();
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceivePlayRespawnEffects();
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceiveLevelUp(AActor* LeveledActor, int32 NewLevel, float LevelupTimeStamp);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceiveDamageReceived(float CurrentHP, float MaxHP, const FDamageContext& DamageContext);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceiveDamageInstigated(const FDamageContext& DamageContext);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceiveTeamChanged(ETeam Team);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraCharacterEvents")
    void ReceiveDied(const FDamageContext& DamageContext);

	virtual void OnRep_PlayerState() override;
};
