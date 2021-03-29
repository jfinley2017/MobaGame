// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilities/Public/GameplayEffectTypes.h"
#include "BasicAttackComponent.generated.h"

class UAnimMontage;

// This effectively lets us slow down the anim, so that the cooldown is up <ATTACK_COMBO_BUFFER> seconds before the current animation
// finishes. Helps to smooth out auto attack chaining. 
//The downside is that we have to allow auto attacks to be cancelled early upon re-activation
#define ATTACK_COMBO_BUFFER 0.3f

USTRUCT(BlueprintType)
struct FBasicAttackAnimationHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BasicAttackEffect")
    UAnimMontage* AttackAnimation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BasicAttackEffect")
    UAnimMontage* AttackAnimationRecovery = nullptr;

	// The max playrate that we should play animations at for this unit
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BasicAttackEffect")
	float MaxAnimPlayrate = 1.5f;

};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), hideCategories = ("Sockets"))
class AGORA_API UBasicAttackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBasicAttackComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	bool bRanged;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
    TArray<FBasicAttackAnimationHandle> ComboSequence;

    // This value is not replicated, and should be used for visual effects only.
    // !!!DO NOT RELY ON THIS FOR ANY GAMEPLAY RELATED THINGS!!!
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "BasicAttack")
    uint8 CurrentComboIndex = 0;

    void SetComboIndex(uint8 NewIndex);    

	//Melee GCs

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "BasicAttack")
	FGameplayTag MeleeHitImpactCue;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "BasicAttack")
	FGameplayTag MeleeMissCue;

	//Ranged GCs

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "BasicAttack")
	FGameplayTag RangedSpawnCue;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "BasicAttack")
	FGameplayTag RangedHitWorldImpactCue;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "BasicAttack")
	FGameplayTag RangedHitCharacterImpactCue;

	// Effect played on the character that was hit, looks like post processing
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "BasicAttack")
	FGameplayTag CameraImpactCue;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "BasicAttack")
	FGameplayTag RangedMissCue;

public:

    UFUNCTION(BlueprintCallable, Category = "BasicAttack")
    void ResetCombo();

    UFUNCTION(BlueprintPure, Category = "BasicAttack")
    uint8 GetCurrentComboIndex() { return CurrentComboIndex; }

	UFUNCTION(BlueprintPure, Category = "BasicAttack")
	uint8 GetPreviousComboIndex();

    // returns animation data for the current combo index that we are on. 
    // optionally progresses the montage AFTER grabbing the animation data
    UFUNCTION(BlueprintCallable, Category = "BasicAttack")
    FBasicAttackAnimationHandle GetCurrentAttackMontage(bool bProgressCombo);

	UFUNCTION(BlueprintPure, Category = "BasicAttack")
	float GetSuggestedMaxAnimPlayRate();

    UFUNCTION(BlueprintPure, Category = "BasicAttack")
    float GetCurrentBasicAnimPlayrate();

};