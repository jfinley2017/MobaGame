// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayEffect.h"
#include "AttributeSet.h" // FGameplayAttribute
#include "AgoraTypes.h" // EAbilityInput
#include "Text.h"
#include "TextProperty.h"
#include "Sound/SoundCue.h"

#include "AgoraDataSingleton.generated.h"

class UAgoraUIBlueprintFunctionLibrary;
class USoundMix;
class USoundClass;

/**
 * Contains data about things which have a more global reach. Extend in blueprint for ease of access (which means this specific class is useless, it's the child you're after).
 * Should be more or less strictly data. There are a few times where some logic is acceptable, but it really should be kept to a minimum. 
 * Treat as a Data Access Object.
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class AGORA_API UAgoraDataSingleton : public UObject
{
	GENERATED_BODY()
	
public:

    UAgoraDataSingleton(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "AgoraDataSingleton")
	USoundMix* GetGlobalSoundMix();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AgoraDataSingleton|Audio")
	USoundClass* MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AgoraDataSingleton|Audio")
	USoundClass* EffectsSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AgoraDataSingleton|Audio")
	USoundClass* MusicSoundClass;


    /**
     * Looks in to the AttributeDataTable, returning the icon associated with @Attribute
     * @Return DefaultBrokenImage if a row associated with @Attribute is not found, DefaultAttributeImage if the image in the row is nullptr
     */
    UFUNCTION(BlueprintPure, Category = "AgoraDataSingleton")
    UTexture2D* GetAttributeIcon(const FGameplayAttribute& Attribute);

    /**
     * Looks in to the AttributeDataTAble, returning the displayname @OutAttributeDisplayName associated with @Attribute
     * Returns NULLSTRING if the row associated with @Attribute was not found, or INDEVSTRING if there was no DisplayName supplied in the row.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraDataSingleton")
    void GetAttributeDisplayName(const FGameplayAttribute& Attribute, FText& OutAttribudeDisplayName);

    /**
     * Looks in to the AttributeDataTable, returning the description @OutAttributeDescription associated with @Attribute
     * Returns NULLSTRING if the row associated with @Attribute was not found, or INDEVSTRING if there was no description supplied in the row.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraDataSingleton")
    void GetAttributeDescription(const FGameplayAttribute& Attribute, FText& OutAttributeDescription);

    UFUNCTION(BlueprintPure, Category = "AgoraDataSingleton")
    FLinearColor GetColorFor(const FName& Element, bool EnsureValid);

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "AgoraDataSingleton|Data")
    UDataTable* UIColorData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|GameplayEffects")
    TSubclassOf<UGameplayEffect> SetByCallerDamageEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|GameplayEffects")
    TSubclassOf<UGameplayEffect> BaseStatsOverrideEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|GameplayEffects")
	TSubclassOf<UGameplayEffect> BaseStatsAddEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|GameplayEffects")
    TSubclassOf<UGameplayEffect> DeathGameplayEffect;

	/**
	 * Made for jungle camp minion resets, possibly relevant for some other things
	 * Should handle things like clearing status tags as well as healing
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|GameplayEffects")
	TSubclassOf<UGameplayEffect> FullHealResetEffect;

    /**
     * The image representing an empty item slot
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|Icons")
    UTexture2D* EmptyItemImage;

    /**
     * The sound cue to be played when we successfully buy an item
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|Sounds")
    USoundCue* BuyItemSuccess;

    /**
     * The sound cue to be played when we successfully sell an item
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|Sounds")
    USoundCue* SellItemSuccess;

    /**
     * The sound cue to be played when an in-game item is clicked
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|Sounds")
    USoundCue* InGameElementClicked;

    /**
     * The standard image for broken things. Different than the standard image for things that are in development.
     * Used to represent that something went wrong here.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|Icons")
    UTexture2D* DefaultBrokenImage;

    /**
     * The standard image for things that are currently in development. Different than an error.
     * "This succeeded, but we don't have any data for this" type situation
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|Icons")
	UTexture2D* DefaultInDevelopmentImage;

	/**
	 * The standard image for missing attributes icons that are in-development
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|Icons")
	UTexture2D* DefaultAttributeImage;

	/**
	 * The standard image for missing abilities icons that are in-development
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|Icons")
    UTexture2D* DefaultAbilityImage;

    /**
     * Attribute Data. Things like "What icon should I have, what description text" etc.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraDataSingleton|Data")
    UDataTable* AttributeDataTable;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AgoraDataSingleton|Audio")
	USoundMix* GlobalSoundMix;

	friend class UAgoraUIBlueprintFunctionLibrary;
	UPROPERTY(EditDefaultsOnly, Category = "AgoraDataSingleton|DisplayNameMapping")
	TMap<FName, FText> AbilityStatNameMap;

	UPROPERTY(EditDefaultsOnly, Category = "AgoraDataSingleton|DisplayNameMapping")
	TMap<EAbilityInput, FText> AbilityInputNameMap;
};
