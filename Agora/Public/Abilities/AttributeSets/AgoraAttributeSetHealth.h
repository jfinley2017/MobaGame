// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "AgoraAttributeSetBase.h"
#include "AgoraTypes.h" // FDamageContext
#include "AgoraAttributeSetHealth.generated.h"

class APlayerState;

// Time in seconds before an instance of damage becomes stale
// Used to prevent damage events from playing on clients who get a damage event that happened some time ago
#define DAMAGE_STALE_INTERVAL 5.0

/**
 * Wraps a FDamageContext reference with a timestamp. Used to determine if damage is fresh or not
 */
USTRUCT(BlueprintType)
struct FDamageContextHandle
{
    GENERATED_BODY()

    FDamageContextHandle() {}

    FDamageContextHandle(float InTimestamp, FDamageContext InDamageContext)
        : Time(InTimestamp), DamageContext(InDamageContext) {}

    UPROPERTY(BlueprintReadOnly, Category = "DamageEvent")
    float Time = -1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "DamageEvent")
    FDamageContext DamageContext;

    // The default time is a trash value, DamageContexts can be bad
    bool IsValid()
    {
        return Time > 0.0f && DamageContext.IsValid();
    }

    // Determines whether or not this event is stale for a specified time
    bool IsStale(float CurrentTime)
    {
        return CurrentTime > Time || Time <= 0.0f || CurrentTime - Time >= DAMAGE_STALE_INTERVAL;
    }

    // Invalidates this damage event
    void MarkStale()
    {
        Time = -1.0f;
    }

};

/**
 * AgoraAttributeSetHealth
 * Holds Health/MaxHealth, notifys owner when dead
 * Notifications work on both client and server, out of the box via replicated properties.
 */
UCLASS()
class AGORA_API UAgoraAttributeSetHealth : public UAgoraAttributeSetBase
{
	GENERATED_BODY()
	
public:

    UAgoraAttributeSetHealth();

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;

    //////////////////////////////////////////////////////////////////////////
    // Events                                                                  

    // Called when damage is taken. ONLY BROADCASTED ON SERVER
    UPROPERTY(BlueprintAssignable, Category = "AttributeSetBase")
    FDiedSignature OnDeath;

    //////////////////////////////////////////////////////////////////////////
    // Attributes 

    // Health, when this reaches 0, the pawn dies
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "AttributeSetBase")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetHealth, Health);
    UFUNCTION()
    void OnRep_Health();

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth,  Category = "AttributeSetBase")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetHealth, MaxHealth);
    UFUNCTION()
    void OnRep_MaxHealth();

    // Health Regen, how much health is restored per tick, where a tick is defined as <todo>
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegen, Category = "AttributeSetBase")
    FGameplayAttributeData HealthRegen;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetHealth, HealthRegen);
    UFUNCTION()
    void OnRep_HealthRegen();

    //-----------------------------------------------------------------------
    // Damage modifying attributes

    // Physical defense, reduces physical damage taken
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PhysicalDefense, Category = "AttributeSetBase")
    FGameplayAttributeData PhysicalDefense;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetHealth, PhysicalDefense);
    UFUNCTION()
    void OnRep_PhysicalDefense();

    // Magical defense, reduces magical damage taken
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MagicalDefense, Category = "AttributeSetBase")
    FGameplayAttributeData MagicalDefense;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetHealth, MagicalDefense);
    UFUNCTION()
    void OnRep_MagicalDefense();

    //-------------------------------------------------------------------------
    // Meta-Attributes

    // Used to collect damage from various sources, gets 'eaten' post-mod and converted into health changes
    UPROPERTY(BlueprintReadOnly, Category = "AttributeSetHealth")
    FGameplayAttributeData Damage;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetHealth, Damage);

    // Same as damage, but for healing
    UPROPERTY(BlueprintReadOnly, Category = "AttributeSetBase", meta = (HideFromLevelInfos))
    FGameplayAttributeData Healing;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetHealth, Healing);

    //////////////////////////////////////////////////////////////////////////
    // RepVars
    //////////////////////////////////////////////////////////////////////////

    UPROPERTY(ReplicatedUsing = OnRep_LastTakeHitInfo)
    FDamageContextHandle LastTakeHitInfo;

    UFUNCTION()
    void OnRep_LastTakeHitInfo();


protected:

    //////////////////////////////////////////////////////////////////////////
    // Utility functions
    
    /** 
     * Encapsulates a change to our Damage attribute. Handles notification of observers 
     */
    void TakeDamage(const struct FGameplayEffectModCallbackData& Data);

    /** 
     * Handles the case where an actor has killed us 
     */
    void HandleDeath(AActor* Killer, AActor* Killed, FGameplayEventData& DamageGameplayEvent);

    /**
     * Sets and replicates LastHitInfo
     */
    void SetHitInfoForReplication(const FDamageContext& DamageContext);


    /**
     * Retrieves PlayerState from an actor. Returns none if no playerstate found
     */
    APlayerState* GetPlayerState(AActor* Actor);

};
