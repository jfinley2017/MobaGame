// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h" // Needed for UStruct/etc
#include "UObject/Object.h" // Needed for UStruct/etc
#include "UObject/Class.h" // Needed for UStruct/etc
#include "Templates/SubclassOf.h" // Needed for UStruct/etc
#include "Engine/NetSerialization.h" // Needed for UStruct/etc

#include "GameplayEffectTypes.h" // FOnAttributeChangeData
#include "Agora.h" // Printing DamageContext string
#include "GameplayAbility.h"

#include "AgoraTypes.generated.h"

//////////////////////////////////////////////////////////////////////////
// Attributes

/**
 * Represents the display data of an attribute. Effectively maps AttributeName -> visuals
 * RowName: the name of the attribute. The same as FGameplayAttributeData->GetName()
 */
USTRUCT(BlueprintType)
struct FAgoraAttributeDisplayData : public FTableRowBase
{
    GENERATED_BODY()

public:

    FAgoraAttributeDisplayData()
    {
    }

    /** The display name associated with this attribute, generally more readable than Name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttributeDisplayData")
    FText DisplayName;

    /** A more indepth description about this attribute. What does it do? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttributeDisplayData")
    FText Description;

    /** The image associated with this attribute */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttributeDisplayData")
    UTexture2D* Icon;

};

//////////////////////////////////////////////////////////////////////////
// Damage

/**
 * Types of damage present. 
 * May end up deprecating this, as maintaining damage types could prove to be cumbersome (violation of open-closed principle)
 */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
    DMGInvalid,
    DMGMagical,
    DMGPhysical,
    DMGTrue
};

/**
 * Struct which wraps damage events. Usually generated from GameplayEffects. Helps to limit scope of a GameplayEffect, and also expose all damage- related values to BP.
 */
USTRUCT(BlueprintType)
struct FDamageContext
{
    GENERATED_BODY()

    // Default constructor, as required
    FDamageContext() {}

    FDamageContext(const FGameplayEventData& GameplayEventData);
  
    // Constructor for lazy people who hate seeing potentially 5+ lines of setup
    FDamageContext(AActor* DmgedActor, AActor* DmgInstigator, float DmgDone, EDamageType DmgType, FGameplayEffectContextHandle DmgEffectContext)
        : DamagedActor(DmgedActor), DamageInstigator(DmgInstigator), Magnitude(DmgDone), DamageType(DmgType), DamagingEffectContext(DmgEffectContext) {}

    UPROPERTY(BlueprintReadWrite, Category = "DamageContext")
    AActor* DamagedActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "DamageContext")
    AActor* DamageInstigator = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "DamageContext")
    float Magnitude = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "DamageContext")
    EDamageType DamageType = EDamageType::DMGInvalid;

    UPROPERTY(BlueprintReadWrite, Category = "DamageContext")
    FGameplayEffectContextHandle DamagingEffectContext;

    bool IsValid() const
    {
        return DamagedActor && DamageInstigator && Magnitude > 0.0f && DamageType != EDamageType::DMGInvalid;
    }
    
    // Implemented in .cpp to avoid additional includes
    FString ToString();
};


/**
 * Custom GameplayEffectContext used for passing additional data around in a GameplayEffectContext.
 * This is replicated, so it's worth not passing massive amounts of data around.
 */
USTRUCT(BlueprintType)
struct AGORA_API FAgoraGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_USTRUCT_BODY()

public:

    FAgoraGameplayEffectContext() 
    {
        ContextTags = FGameplayTagContainer::EmptyContainer;
    }

    virtual ~FAgoraGameplayEffectContext() {}

    /** Appends the context tags to OutContextTags, changes will not reflect on internal ContextTags. Clears @OutContextTags before appending */
    virtual void GetContextTags(FGameplayTagContainer& OutContextTags) const
    {
        OutContextTags.Reset();
        OutContextTags.AppendTags(ContextTags);
    }

    /** Returns ContextTags as a reference, changes will effect context tags */
    virtual FGameplayTagContainer* GetContextTagsMutable()
    {
        return &ContextTags;
    }

    /**
    * Creates a copy of this context, used to duplicate for later modifications (esp regarding applying GEs to target data)
    * Subclasses must override this.
    */
    virtual FGameplayEffectContext* Duplicate() const override
    {
        FAgoraGameplayEffectContext* NewContext = new FAgoraGameplayEffectContext();
        *NewContext = *this;
        NewContext->AddActors(Actors);
        if (GetHitResult())
        {
            // Does a deep copy of the hit result
            NewContext->AddHitResult(*GetHitResult(), true);
        }
        return NewContext;
    }

    /** Custom serialization, subclasses must override this */
    virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

    /** Returns the actual struct used for serialization, subclasses must override this! */
    virtual UScriptStruct* GetScriptStruct() const 
    {
        return FAgoraGameplayEffectContext::StaticStruct();
    }

protected:

    /** A list of additional tags, can be used to provide additional context about this context */
    FGameplayTagContainer ContextTags;

};

template<>
struct TStructOpsTypeTraits< FAgoraGameplayEffectContext > : public TStructOpsTypeTraitsBase2< FAgoraGameplayEffectContext >
{
    enum
    {
        WithNetSerializer = true,
        WithCopy = true		// Necessary so that TSharedPtr<FHitResult> Data is copied around
    };
};

// Enums
UENUM(BlueprintType)
enum class ELaneEnum : uint8 {
	LE_lane0 = 0	UMETA(DisplayName = "Lane 0"),
	LE_lane1		UMETA(DisplayName = "Lane 1"),
	LE_lane2		UMETA(DisplayName = "Lane 2"),
	Num_Lanes		UMETA(Hidden = "true")
};

UENUM(BlueprintType)
enum class ETowerType : uint8
{
	Invalid UMETA(DisplayName = "Invalid"),
	T1Tower UMETA(DisplayName = "T1 tower"),
	T2Tower UMETA(DisplayName = "T2 tower"),
	Inhibitor UMETA(DisplayName = "Inhibitor"),
	Core UMETA(DisplayName = "Core") // we can add inhibitor + core towers later if we want
};

/**
 * For AgoraCharacterBase
 * As shown in https://wiki.unrealengine.com/GameplayAbilities_and_You
 * Maps inputs to abilities'
 * Any abilities which must be referenced in UI belong in here
 */
UENUM(BlueprintType)
enum class EAbilityInput : uint8
{
	Passive UMETA(DisplayName = "Passive"), //0
	AbilityPrimary UMETA(DisplayName = "AbilityPrimary (LMB)"), // 1
	AbilityAlternate UMETA(DisplayName = "AbilitySecondary (RMB)"), // 2
	AbilityOne UMETA(DisplayName = "AbilityOne (Q)"), // 3
	AbilityTwo UMETA(DisplayName = "AbilityTwo (E)"), // 4
	AbilityUltimate UMETA(DisplayName = "AbilityUltimate (R)"), // 5
    Jump UMETA(DisplayName = "Jump (Space)"),
    Recall UMETA(DisplayName = "Recall (B)")
};

//Rules governing how a target actor's rotation should be.
UENUM(BlueprintType)
enum class ETargetActorRotationType : uint8
{
	//Use the rotation of the actor itself.
	//ActorRotation UMETA(DisplayName = "Use Actor Rotation"),
	//Use the rotation of the actor's looking point.
	LookingRotation UMETA(DisplayName = "Use Looking Rotation"),
};

//Rules governing how a target actor's location should be.
UENUM(BlueprintType)
enum class ETargetActorLocationType : uint8
{
	//Use a provided vector as a relative offset to the actor's location.
	LocationRelative UMETA(DisplayName = "Use Actor Location Relative"),
	//Use the location of a provided socket.
	SocketAttachment UMETA(DisplayName = "Use Socket Attachment"),
};

//ALS Enums

UENUM(BlueprintType)
enum class EALS_Gait : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting"),
};

UENUM(BlueprintType)
enum class EALS_MovementMode : uint8
{
	None UMETA(DisplayName = "None"),
	Grounded UMETA(DisplayName = "Grounded"),
	Falling UMETA(DisplayName = "Falling"),
};

UENUM(BlueprintType)
enum class EALS_RotationMode : uint8
{
	LookingDirection UMETA(DisplayName = "LookingDirection"),
};

UENUM(BlueprintType)
enum class EALS_Stance : uint8
{
	Standing UMETA(DisplayName = "Standing"),
};

UENUM(BlueprintType)
enum class EALS_ViewMode : uint8
{
	ThirdPerson UMETA(DisplayName = "ThirdPerson"),
};

UENUM(BlueprintType)
enum class ECardinalDirection : uint8
{
	North UMETA(DisplayName = "North"),
	East UMETA(DisplayName = "East"),
	West UMETA(DisplayName = "West"),
	South UMETA(DisplayName = "South"),
};

UENUM(BlueprintType)
enum class EFootstepType : uint8
{
	Step UMETA(DisplayName = "Step"),
	Walk UMETA(DisplayName = "Walk"),
	Run UMETA(DisplayName = "Run"),
	Sprint UMETA(DisplayName = "Sprint"),
	Pivot UMETA(DisplayName = "Pivot"),
	Jump UMETA(DisplayName = "Jump"),
	Land UMETA(DisplayName = "Land"),
};

UENUM(BlueprintType)
enum class EIdleEntryState : uint8
{
	N_Idle UMETA(DisplayName = "N_Idle"),
	LF_Idle UMETA(DisplayName = "LF_Idle"),
	RF_Idle UMETA(DisplayName = "RF_Idle"),
};

UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	None UMETA(DisplayName = "None"),
	NotMoving UMETA(DisplayName = "NotMoving"),
	Moving UMETA(DisplayName = "Moving"),
	Pivot UMETA(DisplayName = "Pivot"),
	Stopping UMETA(DisplayName = "Stopping"),
};

UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	Forwards UMETA(DisplayName = "Forwards"),
	Backwards UMETA(DisplayName = "Backwards"),
};



//ALS Structs
USTRUCT(BlueprintType)
struct FAgoraALS_PivotParams
{
	GENERATED_BODY()

	FAgoraALS_PivotParams() {}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALSPivotParams")
	float PivotDirection = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALSPivotParams")
	EMovementDirection CompletedMovementDirection = EMovementDirection::Forwards;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALSPivotParams")
	float CompletedStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALSPivotParams")
	EMovementDirection InterruptedMovementDirection = EMovementDirection::Forwards;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALSPivotParams")
	float InterruptedStartTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FAgoraTurnAnims
{
	GENERATED_BODY()

	FAgoraTurnAnims() {}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TurnAnims")
	UAnimMontage* Turn_L_Anim = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TurnAnims")
	UAnimMontage* Turn_R_Anim = nullptr;
};


// Damage Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamageInstigatedSignature, const FDamageContext&, DamageContext);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDamageReceivedSignature, float, CurrHealth, float, MaxHealth, const FDamageContext&, DamageContext);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiedSignature, const FDamageContext&, KillingBlow);

