// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSets/AgoraAttributeSetBase.h"

#include "AgoraAttributeSetStatsBase.generated.h"

/**
 * Stats gained from items which don't quite fit into other categories. A lot of damage related stats. In need for a rename
 */
UCLASS()
class AGORA_API UAgoraAttributeSetStatsBase : public UAgoraAttributeSetBase
{
	GENERATED_BODY()
	
public:

    UAgoraAttributeSetStatsBase();

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;

    // Returns the number of attacks this unit should be doing PER SECOND. See description of @BaseAttackSpeed
    UFUNCTION(BlueprintPure, Category = "AttackSpeed")
    virtual float GetModifiedAttackSpeed();

    // Helper function that returns the cooldown period of a basic attack. Takes into account slows and bonuses
    // Virtual, in case some unit wishes to use some other way to calculate their attack cooldown
    UFUNCTION(BlueprintPure, Category = "AttackSpeed")
    virtual float CalculateBasicAttackCooldown() const;

    //////////////////////////////////////////////////////////////////////////
    // Attributes

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Mana, BlueprintReadWrite, Category = "AttributeSetBase")
    FGameplayAttributeData Mana;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, Mana);
    UFUNCTION()
    void OnRep_Mana();

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_MaxMana, BlueprintReadWrite, Category = "AttributeSetBase")
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, MaxMana);
    UFUNCTION()
    void OnRep_MaxMana();

    /** Mana Regen */
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_ManaRegen, BlueprintReadWrite, Category = "AttributeSetBase")
    FGameplayAttributeData ManaRegen;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, ManaRegen);
    UFUNCTION()
    void OnRep_ManaRegen();

    // Physical power, used to scale physical damage
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_PhysicalPower, BlueprintReadWrite, Category = "AttributeSetBase")
    FGameplayAttributeData PhysicalPower;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, PhysicalPower);
    UFUNCTION()
    void OnRep_PhysicalPower();

    // Magical power, used to scale magical damage
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_MagicalPower, BlueprintReadWrite, Category = "AttributeSetBase")
    FGameplayAttributeData MagicalPower;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, MagicalPower);
    UFUNCTION()
    void OnRep_MagicalPower();

    // CleavePercentage, how much do my auto attacks cleave.
    // Can be a worthless attribute for some heroes
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CleavePercentage, BlueprintReadWrite, Category = "AttributeSetBase")
    FGameplayAttributeData 	CleavePercentage;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, CleavePercentage);
    UFUNCTION()
    void OnRep_CleavePercentage();

    /** AttackSpeed */
    // How many shots we fire a second 
    // A value of 1.0f means 1 shot per second.
    // A value of .5 means one shot every two seconds.
    // A value of 1.5 means one shot every .6 seconds
    // This value + AttackSpeedModifier results in the effective cooldown of the basic attack ability for every hero
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_AttackSpeedBase, BlueprintReadWrite, Category = "AttributeSetBase")
    FGameplayAttributeData AttackSpeedBase;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, AttackSpeedBase);
    UFUNCTION()
    void OnRep_AttackSpeedBase();

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_AttackSpeedModifier, BlueprintReadWrite, Category = "AttributeSetBase")
    FGameplayAttributeData AttackSpeedModifier;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, AttackSpeedModifier);
    UFUNCTION()
    void OnRep_AttackSpeedModifier();

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CooldownReduction, BlueprintReadWrite, Category = "AttributeSetBase")
	FGameplayAttributeData CooldownReduction;
	ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, CooldownReduction);
	UFUNCTION()
	void OnRep_CooldownReduction();

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CriticalHitChance, BlueprintReadWrite, Category = "AttributeSetStatsBase")
    FGameplayAttributeData CriticalHitChance;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, CriticalHitChance);
    UFUNCTION()
    void OnRep_CriticalHitChance();

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CriticalHitDamageModifier, BlueprintReadWrite, Category = "AttributeSetStatsBase")
    FGameplayAttributeData CriticalHitDamageModifier;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetStatsBase, CriticalHitDamageModifier);
    UFUNCTION()
    void OnRep_CriticalHitDamageModifier();
   

protected:


};
