// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "AgoraAttributeSetBase.h"
#include "AgoraAttributeSetMovement.generated.h"

class ACharacter;

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraAttributeSetMovement : public UAgoraAttributeSetBase
{
	GENERATED_BODY()
	
public:

    UAgoraAttributeSetMovement();

    // UAttributeSet
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;
    // ~UAttributeSet

    // Returns the owning unit's movespeed, after calculating the penalty caused by any applied slows (< @TODO)
    UFUNCTION(BlueprintPure, Category = "MovementSpeed")
    float GetMaxSpeed() const;

    //////////////////////////////////////////////////////////////////////////
    // Attributes

    /** MovementSpeed */
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_MovementSpeedBase, BlueprintReadWrite, Category = "AttributeSetMovement")
    FGameplayAttributeData MovementSpeedBase;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetMovement, MovementSpeedBase);
    UFUNCTION()
    void OnRep_MovementSpeedBase();

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_MovementSpeedBonus, BlueprintReadWrite, Category = "AttributeSetMovement")
    FGameplayAttributeData MovementSpeedBonus;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetMovement, MovementSpeedBonus);
    UFUNCTION()
    void OnRep_MovementSpeedBonus();

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_MovementSpeedSlow, BlueprintReadWrite, Category = "AttributeSetMovement")
    FGameplayAttributeData MovementSpeedSlow;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetMovement, MovementSpeedSlow);
    UFUNCTION()
    void OnRep_MovementSpeedSlow();


protected:

    UPROPERTY()
    ACharacter* CachedAvatarAsCharacter = nullptr;

    float DetermineCurrentMovespeedPenalty() const;
    float DetermineCurrentDirectionalMovespeedPenalty() const;
    float DetermineCurrentMovespeedSlowPenalty() const;

    float StrafePenaltyAmount = .15;
    float BackpedalPenaltyAMount = .15;

};
