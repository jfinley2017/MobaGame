#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Abilities/AttributeSets/AgoraAttributeSetStatsBase.h"
#include "Abilities/AttributeSets/AgoraAttributeSetHealth.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraGameplayTypes.h"
#include "AgoraGameplayAbility.generated.h"

USTRUCT(BlueprintType)
struct FScaledModifier
{
	GENERATED_BODY();

public:

	UPROPERTY(BlueprintReadWrite)
	FName CurveRowName = FName("Damage");

	UPROPERTY(BlueprintReadWrite)
	FGameplayAttribute AttributeModified = UAgoraAttributeSetHealth::GetDamageAttribute();
};

// Log types, to be exposed in blueprint nodes so that the caller can specify which log to use
UENUM()
enum class EAbilityLogType : uint8
{
    AgoraAbility,
    AgoraAbilityVerbose
};

/**
 * Determines how this ability handles calls for it to activate while other targeting actions are taking place
 */
UENUM()
enum class EActivationTargetingRules : uint8
{
	// We can be used while another targeting action is taking place
    ActivatableWhileTargeting, 
	// We can be used while another targeting action is taking place, but we're going to cancel all targeting actions when we activate
    ActivatableWhileTargetingExclusive, 
	// We cannot be used when another targeting action is taking place
    NotActivatableWhileTargeting 
};

class AAgoraCharacterBase;
class UCameraComponent;
class UDataTable;
class UTexture2D;

/**
 * Gives us a chance to override specific functionality before the BP level. All abilities should inhert from here rather than UGameplayAbility.
 */
UCLASS(HideCategories=("Cooldowns"))
class AGORA_API UAgoraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:

	UAgoraGameplayAbility();

    // UGameplayAbility interface
    virtual uint8 GetGameplayTaskDefaultPriority() const { return FGameplayTasks::DefaultPriority; }
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /* = nullptr */, const FGameplayTagContainer* TargetTags /* = nullptr */, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
    virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags /* = nullptr */, const FGameplayTagContainer* TargetTags /* = nullptr */, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */) const override;
    virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
    virtual UGameplayEffect* GetCostGameplayEffect() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
    // ~UGameplayAbility interface
	
	UFUNCTION(BlueprintPure, Category = "AgoraAbility")
	virtual FAgoraAbilityMetadata GetAbilityMetadata();

	// Level is provided because this will be called on the CDO
	// To add more named arguments to the description format text, override this and return the parent
	// result text formatted with whatever extra arguments you want
	UFUNCTION(BlueprintImplementableEvent, Category = "AgoraAbility|Metadata")
	void FormatDescriptionText(int32 Level, const FText& InText, FText& OutText);

	// Level is provided because this will be called on the CDO
	UFUNCTION(BlueprintPure)
	virtual FText GetAbilityDescription(int32 Level);

	//Returns the owning actor as a Pawn reference for BP convenience. Fatally crashes if the owning actor does not implement AgoraUnit.
	UFUNCTION(BlueprintPure, Category = "AgoraAbility")
	APawn* GetOwningActorFromActorInfoAsPawn() const;

	//Returns the owning actor as an AgoraCharacterBase reference for BP convenience. Returns NULL if the cast fails.
	UFUNCTION(BlueprintPure, Category = "AgoraAbility")
	AAgoraCharacterBase* GetOwningActorFromActorInfoAsAgoraCharacter() const;

	//Returns the GASCMC of the owning actor. Fatally asserts whether the owning actor is an AgoraCharacter.
	UFUNCTION(BlueprintPure, Category = "AgoraAbility")
	UGASCharacterMovementComponent* GetOwningActorGASCMC() const;

	UFUNCTION(BlueprintCallable, Category = "AgoraAbility")
	bool CanUpgrade(int32 HeroLevel, int32 SpecAbilityLevel);

	//Returns the owning actor's camera for BP convenience. May return null.
	UFUNCTION(BlueprintPure, Category = "AgoraAbility")
	UCameraComponent* GetCameraComponentFromOwningActor() const;

	//Move and rotate the camera by some relative amount. See also AgoraMoveComponentTo
	UFUNCTION(BlueprintCallable, meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"), Category = "AgoraAbility")
	void MoveCameraTo(FVector TargetRelativeLocation, FRotator TargetRelativeRotation, bool bEaseOut, bool bEaseIn, float OverTime, FLatentActionInfo LatentInfo);
	
	//Reset the camera back to its original position. See also AgoraMoveComponentTo
	UFUNCTION(BlueprintCallable, meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"), Category = "AgoraAbility")
	void ResetCamera(bool bEaseOut, bool bEaseIn, float OverTime, FLatentActionInfo LatentInfo);

    UFUNCTION(BlueprintPure, Category = "AbilityActivation")
    float GetScaledCostMagnitude() const;

	UFUNCTION(BlueprintPure, Category = "AbilityActivation")
	virtual FGameplayAttribute GetCostAttribute() const;

    UFUNCTION(BlueprintPure, Category = "AgoraAbility")
    int32 GetMaxLevel() const;

	UFUNCTION(BlueprintPure, Category = "AgoraAbility")
	UCurveTable* GetAbilityLevelTable() const;

	// Simpler version of getting effect context for instanced abilities
	UFUNCTION(BlueprintCallable, Category = "AgoraAbility")
	FGameplayEffectContextHandle MakeInstancedEffectContext();

	// If this is called on the CDO it will not work
	UFUNCTION(BlueprintPure, Category = "AgoraAbility")
	EAbilityInput GetAbilitySlot() const;

	UFUNCTION(BlueprintPure, Category = "AgoraAbility|Abilities")
	float GetAbilityStat(FName StatRowName) const;

	UFUNCTION(BlueprintPure, Category = "AgoraAbility|Abilities")
	FScalableFloat GetAbilityStatCurve(FName StatRowName) const;

	UFUNCTION(BlueprintPure, Category = "AgoraAbility|Abilities")
	FName GetCooldownRowName() const;

	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	// This can't be BP callable because it's not a USTRUCT
	FGameplayTagCountContainer& GetTagCountContainer();

	UFUNCTION(BlueprintPure, Category = "AgoraAbility|Abilities|Charges")
	int32 GetChargeCount() const;

	// < 0 refreshes all charges. 
	UFUNCTION(BlueprintCallable, Category = "AgoraAbility|Abilities|Charges", meta = (AdvancedDisplay = 0))
	void RefreshCharges(int32 NumCharges = -1);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Initialization|Charges")
	bool bUseCharges = false;

	UPROPERTY(EditDefaultsOnly, Category = "Initialization|Charges", meta = (EditCondition = "bUseCharges"))
	int32 MaxCharges = 0;

	// Useful for storing data on the ability, particularly charges, activation counts, etc.
	FGameplayTagCountContainer TagCountContainer;


	/** This GameplayEffect represents the cooldown. It will be applied when the ability is committed and the ability cannot be used again until it is expired. 
	* This only allows effects that subclassed from CooldownGameplayEffect to make it easier to find
	* Internally this sets the original CooldownGameplayEffectClass
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Initialization|Cooldown")
	TSubclassOf<class UCooldownGameplayEffect> CooldownGEClass;

	// Mostly this is the same as ApplyGameplayEffectToOwner, but accepts some tags that can be granted to owner, and uses TSubclassOf for the effect class instead of GetClass
	virtual FActiveGameplayEffectHandle ApplyGameplayEffectToOwnerWithTags(const FGameplayTagContainer* DynamicGrantedTags, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, TSubclassOf<UGameplayEffect> GameplayEffectClass, int32 Stacks = 1) const;

	// Each index in the array represents an ability level, the value in the array 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization")
	UCurveTable* AbilityLevelTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization|Metadata")
	UDataTable* AbilityMetadataTable;

	// This is the name of the ability in the metadata table
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization|Metadata")
	FName AbilityNameId;

	UFUNCTION(BlueprintPure, meta = (AdvancedDisplay = 2))
	FGameplayEffectSpecHandle SetScaledDamageSpec(FGameplayEffectSpecHandle SpecHandle, EDamageType DamageType = EDamageType::DMGInvalid, FName CoefficientRow = FName("DamageCoefficient"), FName BaseDamageRow = "damage", float Multiplier = 1.0f);

	/**
	* Looks up damage and power scaling from the ability level table and creates a corresponding GE. Coefficient = Scaling from power
	* @param DamageType This determines which damage tag type will be added to the GE
	* @param NumTicks allows you to scale the amount of damage over the amount of ticks that will occur. Damage / NumTicks
	*/
	UFUNCTION(BlueprintPure, meta = (AdvancedDisplay = 1))
	FGameplayEffectSpecHandle GetScaledDamageSpec(EDamageType DamageType = EDamageType::DMGInvalid, FName CoefficientRow = FName("DamageCoefficient"), FName BaseDamageRow = "damage", float Multiplier = 1.0f);

	/**
	 * Apply damage in an area, scaled by level
	 * For effects involving time, use the WaitTicks node and/or UAgoraAbilitySystem::ApplyEffectInArea
	 * @param Area The collision component that determines where the effect is applied
	 * @param CoefficientRow The row in the ability level table to get the power scaling (damage coefficient) value from
	 * @param BaseDamageRow The row in the ability level table to get the damage magnitude from. Will be divided by NumTicks (damage is spread out over the duration of the effect)
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 3), Category = "AgoraAbility|Abilities")
	const TSet<AActor*> ApplyScaledDamageInArea(UPrimitiveComponent* Area, FAgoraSweepQueryParams QuerySweepParams, EDamageType DamageType = EDamageType::DMGInvalid,
		FName CoefficientRow = FName("DamageCoefficient"), FName BaseDamageRow = "damage", float Multiplier = 1.0f);

    ////////////////////////////////////////////////////////////////////////
    // Config
	
	FGameplayTag ChargeTag = UAgoraGameplayTags::Charges();
	
	UPROPERTY(EditDefaultsOnly, Category = "Initialization|Cooldown")
	FGameplayTagContainer CooldownTags;

	// Used to look up the cooldown time (in seconds) by ability level
	UPROPERTY(EditDefaultsOnly, Category = "Initialization|Cooldown")
	FName CooldownRowName = FName("Cooldown");

	// Used to look up the ability cost in the ability level table
	UPROPERTY(EditDefaultsOnly, Category = "Initialization|Cost")
	FName CostRowName = FName("ManaCost");

	// Change this if the ability should cost something besides mana, e.g. health, ammo, etc.
	// Set this to None if the ability should not have a cost
	UPROPERTY(EditDefaultsOnly, Category = "Initialization|Cost")
	FGameplayAttribute CostAttribute = UAgoraAttributeSetStatsBase::GetManaAttribute();

	// If true, ability incurs Global Cooldown
	UPROPERTY(EditDefaultsOnly, Category = "Initialization")
	bool bIncursGlobalCooldown = false;

	UPROPERTY()
	float GlobalCooldownDuration = 0.5f;

    //////////////////////////////////////////////////////////////////////////
    // Tertiary tags
    // Here to avoid frequently required tags from being added/removed every time we create a new ability. It's best to think of these as tags. Trying to prevent accidents from frequen
    // Also can help to encapsulate complex state and handle doing additional things based on the existence of tags (canceling targeting actions)
    // EG: most abilities are going to require the target to be alive. If we can find a better way of applying tags by default, this can go in the trash bin.
    // Missing: some easy way to do CC checks.

    /**
    * Is this ability activatable while another ability is running an async action for the caller? ("casting")
    */
    UPROPERTY(EditDefaultsOnly, Category = "Initialization|AbilityActivation")
    bool bActivatableWhileCasting = false;

    /**
     * Is this ability able to be used while the caller is dead?
     */
    UPROPERTY(EditDefaultsOnly, Category = "Initialization|AbilityActivation")
    bool bActivatableWhileDead = false;

    /** 
    * Can this ability be activated while a targeting task is running for the caller?
    * Helps for situations like preventing an auto attack from happening when we're targeting.
    * May also cause other targeting actions to cancel
    */
    UPROPERTY(EditDefaultsOnly, Category = "Initialization|AbilityActivation")
    EActivationTargetingRules ActivationTargetingRules = EActivationTargetingRules::ActivatableWhileTargetingExclusive;

	//we save the loc/rot of the camera for convenience in MoveCamera/MoveComponent functions
	UPROPERTY(BlueprintReadOnly, Category = "AgoraAbility")
	FVector DefaultHeroCameraRelativeLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraAbility")
	FRotator DefaultHeroCameraRelativeRotation = FRotator::ZeroRotator;

    //////////////////////////////////////////////////////////////////////////
    // Logging

    // Verbosity types cannot (or at the very least, are very hard to) change at runtime, as the macros expect a certain type.

    UFUNCTION(BlueprintCallable, Category = "AgoraAbility")
    void LogAbility(const FString& InString = FString(TEXT("Hello")), EAbilityLogType AbilityLogType = EAbilityLogType::AgoraAbility, bool bPrintToScreen = true, bool bPrintToLog = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);
	
    UFUNCTION(BlueprintCallable, Category = "AgoraAbility")
    void LogAbility_Error(const FString& InString = FString(TEXT("Hello")), EAbilityLogType AbilityLogType = EAbilityLogType::AgoraAbility, bool bPrintToScreen = true, bool bPrintToLog = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);

    UFUNCTION(BlueprintCallable, Category = "AgoraAbility")
    void LogAbility_Warning(const FString& InString = FString(TEXT("Hello")), EAbilityLogType AbilityLogType = EAbilityLogType::AgoraAbility, bool bPrintToScreen = true, bool bPrintToLog = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);

    UFUNCTION(BlueprintCallable, Category = "AgoraAbility")
    void LogAbility_Fatal(const FString& InString = FString(TEXT("Hello")), EAbilityLogType AbilityLogType = EAbilityLogType::AgoraAbility);
};
