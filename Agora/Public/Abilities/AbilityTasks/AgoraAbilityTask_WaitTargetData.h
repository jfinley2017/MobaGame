// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h" // Delegate signatures
#include "Tasks/AbilityTask.h" // the rest of the copy paste, can be removed as we're including WaitTargetData, but i'm leaving it as evidence that I really want to edit engine source
#include "GameplayTagContainer.h" // Applying gameplay tags to owner

#include "AgoraAbilityTask_WaitTargetData.generated.h"

class AAIController;

/**
 * Copy and pasted from AbilityTask_WaitTargetData - Candidate for removal upon engine editing
 * Edited to apply (and remove) certain tags + gameplaycues upon activation.
 */
UCLASS(notplaceable)
class AGORA_API UAgoraAbilityTask_WaitTargetData : public UAbilityTask
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintAssignable)
    FWaitTargetDataDelegate	ValidData;

    UPROPERTY(BlueprintAssignable)
    FWaitTargetDataDelegate	Cancelled;

    UFUNCTION()
    void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);

    UFUNCTION()
    void OnTargetDataReplicatedCancelledCallback();

    UFUNCTION()
    void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data);

    UFUNCTION()
    void OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data);

    /** Spawns target actor and waits for it to return valid data or to be canceled. */
    UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Agora|Tasks")
    static UAgoraAbilityTask_WaitTargetData* AgoraWaitTargetData(
        UGameplayAbility* OwningAbility, 
        FName TaskInstanceName, 
        FGameplayTag GameplayCueTag,
        FGameplayCueParameters GameplayCueParameters,
        FGameplayTagContainer TagsToApplyToOwner,
        TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, 
        TSubclassOf<AGameplayAbilityTargetActor> Class);

    /** Uses specified target actor and waits for it to return valid data or to be canceled. */
    UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Agora|Tasks")
    static UAgoraAbilityTask_WaitTargetData* AgoraWaitTargetDataUsingActor(
        UGameplayAbility* OwningAbility, 
        FName TaskInstanceName, 
        FGameplayTag GameplayCueTag,
        FGameplayCueParameters GameplayCueParameters,
        FGameplayTagContainer TagsToApplyToOwner,
        TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, 
        AGameplayAbilityTargetActor* InTargetActor);

    /** See parent, overridden to apply tags to owner */
    void Activate() override;

    UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Abilities")
    bool BeginSpawningActor(UGameplayAbility* OwningAbility, TSubclassOf<AGameplayAbilityTargetActor> Class, AGameplayAbilityTargetActor*& SpawnedActor);

    UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Abilities")
    void FinishSpawningActor(UGameplayAbility* OwningAbility, AGameplayAbilityTargetActor* SpawnedActor);

    /** Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true. */
    virtual void ExternalConfirm(bool bEndTask) override;

    /** Called when the ability is asked to cancel from an outside node. What this means depends on the individual task. By default, this does nothing other than ending the task. */
    virtual void ExternalCancel() override;

protected:

    bool ShouldSpawnTargetActor() const;
    void InitializeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const;
    void FinalizeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const;
    void RegisterTargetDataCallbacks();
    bool ShouldReplicateDataToServer() const;

    /** See parent, overridden to remove applied tags from owner */
    virtual void OnDestroy(bool AbilityEnded) override;

	// Infers whether we are controlled by an AI
	bool IsAIControlled() const;

    /**
     * The gameplaycue that will be executed for the ability owner (the one targeting)
     */
    UPROPERTY()
    FGameplayTag GameplayCueTag;

    UPROPERTY()
    FGameplayCueParameters GameplayCueParameters;

    /** 
    * A container which holds blueprint specified tags that we're applying to the targeting actor when this task is activated 
    * Tags will be removed when this task ends (is destroyed).
    * Note that tags are applied loosely (eg, not replicated).
    */
    UPROPERTY(BlueprintReadOnly, Category = "WaitTargetData")
    FGameplayTagContainer TagsToApplyToOwner;

    UPROPERTY()
    TSubclassOf<AGameplayAbilityTargetActor> TargetClass;

    /** The TargetActor that we spawned */
    UPROPERTY()
    AGameplayAbilityTargetActor* TargetActor;

    TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType;

    FDelegateHandle OnTargetDataReplicatedCallbackDelegateHandle;

};
