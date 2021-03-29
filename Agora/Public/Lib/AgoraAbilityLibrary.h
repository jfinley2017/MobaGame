// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "GameplayEffectTypes.h"
#include "AgoraGameplayTypes.h"
#include "AgoraAbilityLibrary.generated.h"

class UPrimitiveComponent;
class UGameplayAbility;
class UAbilitySystemComponent;

DECLARE_LOG_CATEGORY_EXTERN(AgoraAbility, Log, All);
DECLARE_STATS_GROUP(TEXT("AgoraAbility"), STATGROUP_AgoraAbility, STATCAT_Advanced);

/**
 * GAS related, blueprint function library
 */
UCLASS()
class AGORA_API UAgoraAbilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	// This exists just because you cannot define UFUNCTIONs in USTRUCTs
	UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary")
	static bool MatchesSweepQueryParams(FAgoraSweepQueryParams Params, AActor* SweepingActor, AActor* SweptActor);

	/**
	 * For a damage version, see UAgoraAbility
	 * @param Area The collision component that determines where the effect is applied
	 * @param Effect GE to apply
	 * @param Duration How long the area will be active. Actors that enter the area will have the effect applied to them
	 * @param bRemoveEffectOnLeaveArea Only relevant if duration > 0. Removes the GE when actors leaves the area
	 */
	UFUNCTION(BlueprintCallable, Category = "Agora|Abilities")
	static const TSet<AActor*> ApplyEffectInArea(UPrimitiveComponent* Area, FGameplayEffectSpecHandle Effect, FAgoraSweepQueryParams QueryParams, AActor* EffectCauser);

	UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary")
	static void GetAbilitiesWithTags(FGameplayTagContainer Tags, const UAbilitySystemComponent* ASC, TArray<UGameplayAbility*>& OutAbilities);

    UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary")
    static TSubclassOf<UGameplayAbility> GetGameplayAbilityClassForInputID(AActor* Actor, EAbilityInput InputID);

    UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary")
    static int32 GetAbilityLevelFromInputID(AActor* Actor, EAbilityInput InputID);

    UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary")
    static bool GetAbilityCooldownForActor(TSubclassOf<UGameplayAbility> Ability, AActor* Actor, float& OutTimeRemaining, float& OutCooldownDuration);

    UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary")
    static bool GetAbilityCost(TSubclassOf<UGameplayAbility> Ability, AActor* Actor, float& Cost, FGameplayAttribute& Attribute);

    UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary")
    static bool CanAbilityBeLeveled(AActor* Actor, TSubclassOf<UGameplayAbility> Ability);

    UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary")
    static int32 GetAbilityLevelFromClass(AActor* Actor, TSubclassOf<UGameplayAbility> Ability);

    UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary")
    static int32 GetAbilityMaxLevelFromClass(TSubclassOf<UGameplayAbility> Ability);

	/////////////////////////////
	// GAS Related

	UFUNCTION(BlueprintCallable, Category = "Agora||AgoraAbilityLibrary|GameplayCues", meta = (AdvancedDisplay = 1))
	static void AddGameplayCueToASC(UAbilitySystemComponent* ASC, const FGameplayTag GameplayCueTag);

	UFUNCTION(BlueprintCallable, Category = "Agora||AgoraAbilityLibrary|GameplayCues", meta = (AdvancedDisplay = 1))
	static void ExecuteGameplayCueOnASC(UAbilitySystemComponent* ASC, const FGameplayTag GameplayCueTag);

	/////////////////////////////
	// UI Related

	// This is TSubclassOf<UGameplayAbility> for convenience, the ability must be a UAgoraAbility
	UFUNCTION(BlueprintPure, Category = "Agora|AgoraAbilityLibrary|Metadata")
	static FAgoraAbilityMetadata GetAbilityMetadata(TSubclassOf<UGameplayAbility> AbilityClass);

	// Originally wanted to put this in ability itself, but cannot make a static bp callable function elsewhere
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static FText GetAbilityDescription(TSubclassOf<UAgoraGameplayAbility> Ability, int32 Level);

	// Checks the actor to figure out which ability class maps to a particular input id
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static TSubclassOf<UAgoraGameplayAbility> GetAbilityClassByInputId(AActor* Actor, EAbilityInput InputId);

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static float GetFloatAttributeChecked(const AActor* Actor, FGameplayAttribute Attribute);

	//Adds a non-replicated tag to the specified ASC. This is just a wrapper around the built-in GAS function, which is not exposed to BP.
	UFUNCTION(BlueprintCallable, Category = "Agora")
		static void AddLooseGameplayTagToASC(const FGameplayTag GameplayTag, UAbilitySystemComponent* AbilitySystemComponent, int32 Count);

	//Removes a non-replicated tag from the specified ASC. This is just a wrapper around the built-in GAS function, which is not exposed to BP.
	UFUNCTION(BlueprintCallable, Category = "Agora")
		static void RemoveLooseGameplayTagFromASC(const FGameplayTag GameplayTag, UAbilitySystemComponent* AbilitySystemComponent, int32 Count);

	// Get the cooldown tags associated with an ability, you must check IsValid on the resulting container
	UFUNCTION(BlueprintCallable, Category = "Agora|AgoraAbilityLibrary|Cooldowns", meta = (DisplayName = "GetCooldownTags"))
	const FGameplayTagContainer BP_GetCooldownTags(TSubclassOf<UGameplayAbility> Ability);

	static const FGameplayTagContainer* GetCooldownTags(TSubclassOf<UGameplayAbility> Ability);

	/**
	 * Sets the damage type on a given spec (magical, physical, true)
	 */
	UFUNCTION(BlueprintCallable)
	static FGameplayEffectSpecHandle SetEffectDamageType(FGameplayEffectSpecHandle Spec, EDamageType DamageType = EDamageType::DMGInvalid);

	/**
	 * @param Damage How much damage to do
	 * @param ScalingValue Multiplier for physical or magical power based on user's stat, if applicable
	 * @param DamageType
	 * @param Context Effect context, from ability or ability system component
	 */
	static FGameplayEffectSpecHandle GetDamageSpec(FGameplayEffectContextHandle Context, float Damage = 100.0, float ScalingValue = 1.0, EDamageType DamageType = EDamageType::DMGInvalid);

	/**
	 * Can be used to create GE's that do damage in addition to other stuff
	 */
	UFUNCTION(BlueprintCallable)
	static FGameplayEffectSpecHandle SetDamageSpecValues(FGameplayEffectSpecHandle SpecHandle, float Damage = 100.0, float ScalingValue = 1.0, EDamageType DamageType = EDamageType::DMGInvalid);

	/**
	 * This is intended for use with BP primarily, especially when you don't have an agora ASC
	 * @param Instigator
	 */
	UFUNCTION(BlueprintCallable)
	static FGameplayEffectSpecHandle GetDamageSpec(UAbilitySystemComponent* AbilitySystem = nullptr, float Damage = 100.0, float ScalingValue = 1.0, EDamageType DamageType = EDamageType::DMGInvalid);

   /**
	* Get a spec handle form ability input
	*/
	UFUNCTION(BlueprintCallable, Category = "Agora|AgoraAbilityLibrary")
	static bool FindAbilitySpecHandleBySlot(EAbilityInput AbilitySlot, UAbilitySystemComponent* ASC, FGameplayAbilitySpecHandle& OutAbilityHandle);

    /**
     * Gives the attribute name from an attribute.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraAbilityLibrary")
    static void GetAttributeName(FGameplayAttribute Attribute, FString& OutAttributeName);

};
