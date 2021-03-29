// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Animation/AnimInstance.h"
#include "AgoraAbilityTask_PlayMontageAndWaitForEvent.generated.h"

class UAgoraAbilitySystemComponent;

/** Delegate type used, EventTag and Payload may be empty if it came from the montage callbacks */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAgoraPlayMontageAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);


UENUM()
enum class EPlayMontageAndWaitForEventTagRemovalRule : uint8
{
    // remove the supplied tags when the event was broadcasted, note that if an event is never hit the tags will never be removed
    EventBroadcast,
    // remove the supplied tags when the montage ends for any reason
    MontageEnding,
    // will remove the supplied tags when an event is broadcasted, if no event happens the tags will be removed when the montage ends in any way
    Dynamic,
    // the calling ability controls the removal of added tags, we are simply placing them
    Custom 
};

// Taken from ARPG example: https://docs.unrealengine.com/en-us/Resources/SampleGames/ARPG
// Modified to allow for a tag to be added for the duration of this

/**
 * This task combines PlayMontageAndWait and WaitForEvent into one task, so you can wait for multiple types of activations such as from a melee combo
 * Much of this code is copied from one of those two ability tasks
 * This is a good task to look at as an example when creating game-specific tasks
 * It is expected that each game will have a set of game-specific tasks to do what they want
 */
UCLASS()
class AGORA_API UAgoraAbilityTask_PlayMontageAndWaitForEvent : public UAbilityTask
{
    GENERATED_BODY()

public:
    // Constructor and overrides
    UAgoraAbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer);
    virtual void Activate() override;
    virtual void ExternalCancel() override;
    virtual FString GetDebugString() const override;
    virtual void OnDestroy(bool AbilityEnded) override;

    /** The montage completely finished playing */
    UPROPERTY(BlueprintAssignable)
    FAgoraPlayMontageAndWaitForEventDelegate OnCompleted;

    /** The montage started blending out */
    UPROPERTY(BlueprintAssignable)
    FAgoraPlayMontageAndWaitForEventDelegate OnBlendOut;

    /** The montage was interrupted */
    UPROPERTY(BlueprintAssignable)
    FAgoraPlayMontageAndWaitForEventDelegate OnInterrupted;

    /** The ability task was explicitly cancelled by another ability */
    UPROPERTY(BlueprintAssignable)
    FAgoraPlayMontageAndWaitForEventDelegate OnCancelled;

    /** One of the triggering gameplay events happened */
    UPROPERTY(BlueprintAssignable)
    FAgoraPlayMontageAndWaitForEventDelegate EventReceived;

    /**
     * Play a montage and wait for it end. If a gameplay event happens that matches EventTags (or EventTags is empty), the EventReceived delegate will fire with a tag and event data.
     * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
     * On normal execution, OnBlendOut is called when the montage is blending out, and OnCompleted when it is completely done playing
     * OnInterrupted is called if another montage overwrites this, and OnCancelled is called if the ability or task is cancelled
     *
     * @param TaskInstanceName Set to override the name of this task, for later querying
     * @param MontageToPlay The montage to play on the character
     * @param EventTags Any gameplay events matching this tag will activate the EventReceived callback. If empty, all events will trigger callback
     * @param Rate Change to play the montage faster or slower
     * @param bStopWhenAbilityEnds If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
     * @param AnimRootMotionTranslationScale Change to modify size of root motion or set to 0 to block it entirely
     */
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
        static UAgoraAbilityTask_PlayMontageAndWaitForEvent* PlayMontageAndWaitForEvent(
            UGameplayAbility* OwningAbility,
            FName TaskInstanceName,
            UAnimMontage* MontageToPlay,
            FGameplayTagContainer EventTags,
            EPlayMontageAndWaitForEventTagRemovalRule TagRemovalRule,
            FGameplayTagContainer TagsToApplyToOwner,
            float Rate = 1.f,
            FName StartSection = NAME_None,
            bool bStopWhenAbilityEnds = true,
            float AnimRootMotionTranslationScale = 1.f);

private:

    /** Montage that is playing */
    UPROPERTY()
    UAnimMontage* MontageToPlay;

    /** List of tags to match against gameplay events */
    UPROPERTY()
    FGameplayTagContainer EventTags;

    /** 
    * Tags that are added for the duration of this task, removed as specified in @TagRemovalRule 
    * Note that these are added as loose tags. So they are not properly replicated.
    */
    UPROPERTY()
    FGameplayTagContainer AppliedTags;

    /** When should we be removing AppliedTags */
    UPROPERTY()
    EPlayMontageAndWaitForEventTagRemovalRule TagRemovalRule = EPlayMontageAndWaitForEventTagRemovalRule::Dynamic;

    /** Used to determine whether or not AppliedTags have been removed, in case of a dynamic removal rule we don't want to remove twice */
    UPROPERTY()
    bool bHasRemovedTags = false;

    /** Playback rate */
    UPROPERTY()
    float Rate;

    /** Section to start montage from */
    UPROPERTY()
    FName StartSection;

    /** Modifies how root motion movement to apply */
    UPROPERTY()
    float AnimRootMotionTranslationScale;

    /** Rather montage should be aborted if ability ends */
    UPROPERTY()
    bool bStopWhenAbilityEnds;

    /** Checks if the ability is playing a montage and stops that montage, returns true if a montage was stopped, false if not. */
    bool StopPlayingMontage();

    /** Returns our ability system component */
    UAgoraAbilitySystemComponent* GetTargetASC();

    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    void OnAbilityCancelled();
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);
    void RemoveAppliedTags();

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    FOnMontageEnded MontageEndedDelegate;
    FDelegateHandle CancelledHandle;
    FDelegateHandle EventHandle;
};