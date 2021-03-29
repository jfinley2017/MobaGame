// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.


#include "AgoraAbilityTask_PlayMontageAndWaitForEvent.h"
#include "Components/AgoraAbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimInstance.h"

#include "Agora.h" // Logging

UAgoraAbilityTask_PlayMontageAndWaitForEvent::UAgoraAbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Rate = 1.f;
    bStopWhenAbilityEnds = true;
}

UAgoraAbilityTask_PlayMontageAndWaitForEvent* UAgoraAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(UGameplayAbility* OwningAbility,
    FName TaskInstanceName, UAnimMontage* MontageToPlay, FGameplayTagContainer EventTags, EPlayMontageAndWaitForEventTagRemovalRule TagRemovalRule, FGameplayTagContainer TagsToApplyToOwner, float Rate, FName StartSection, bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale)
{
    UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

    UAgoraAbilityTask_PlayMontageAndWaitForEvent* MyObj = NewAbilityTask<UAgoraAbilityTask_PlayMontageAndWaitForEvent>(OwningAbility, TaskInstanceName);
    MyObj->MontageToPlay = MontageToPlay;
    MyObj->EventTags = EventTags;
    MyObj->TagRemovalRule = TagRemovalRule;
    MyObj->AppliedTags = TagsToApplyToOwner;
    MyObj->Rate = Rate;
    MyObj->StartSection = StartSection;
    MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
    MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;

    return MyObj;
}

void UAgoraAbilityTask_PlayMontageAndWaitForEvent::Activate()
{
    if (Ability == nullptr)
    {
        return;
    }

    bool bPlayedMontage = false;
    UAgoraAbilitySystemComponent* ASC = GetTargetASC();

    if (ASC)
    {
        const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
        UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
        if (AnimInstance != nullptr)
        {
            // Bind to event callback
            EventHandle = ASC->AddGameplayEventTagContainerDelegate(EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UAgoraAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent));

            if (ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection) > 0.f)
            {
                // Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
                if (ShouldBroadcastAbilityTaskDelegates() == false)
                {
                    return;
                }

                // Add Tags
                AbilitySystemComponent->AddLooseGameplayTags(AppliedTags);

                // Bind to anim events
                CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UAgoraAbilityTask_PlayMontageAndWaitForEvent::OnAbilityCancelled);

                // blend out
                BlendingOutDelegate.BindUObject(this, &UAgoraAbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut);
                AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

                // montage end
                MontageEndedDelegate.BindUObject(this, &UAgoraAbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded);
                AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

                ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
                if (Character && (Character->Role == ROLE_Authority ||
                    (Character->Role == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
                {
                    Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
                }

                bPlayedMontage = true;
            }
        }
        else
        {
            ABILITY_LOG(Warning, TEXT("UAgoraAbilityTask_PlayMontageAndWaitForEvent call to PlayMontage failed!"));
        }
    }
    else
    {
        ABILITY_LOG(Warning, TEXT("UAgoraAbilityTask_PlayMontageAndWaitForEvent called on invalid AbilitySystemComponent"));
    }

    if (!bPlayedMontage)
    {
        ABILITY_LOG(Warning, TEXT("UAgoraAbilityTask_PlayMontageAndWaitForEvent called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
        RemoveAppliedTags();
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }

    SetWaitingOnAvatar();
}

void UAgoraAbilityTask_PlayMontageAndWaitForEvent::ExternalCancel()
{
    check(AbilitySystemComponent);

    OnAbilityCancelled();

    Super::ExternalCancel();
}

void UAgoraAbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
    {
        if (Montage == MontageToPlay)
        {
            AbilitySystemComponent->ClearAnimatingAbility(Ability);

            // Reset AnimRootMotionTranslationScale
            ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
            if (Character && (Character->Role == ROLE_Authority ||
                (Character->Role == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
            {
                Character->SetAnimRootMotionTranslationScale(1.f);
            }

        }
    }

    if (TagRemovalRule == EPlayMontageAndWaitForEventTagRemovalRule::MontageEnding || TagRemovalRule == EPlayMontageAndWaitForEventTagRemovalRule::Dynamic)
    {
        RemoveAppliedTags();
    }

    if (bInterrupted)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }
    else
    {
        
        if (ShouldBroadcastAbilityTaskDelegates())
        {

            OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }
}

void UAgoraAbilityTask_PlayMontageAndWaitForEvent::OnAbilityCancelled()
{
    // TODO: Merge this fix back to engine, it was calling the wrong callback

    if (StopPlayingMontage())
    {
        // If the ability is canceled, we're removing the tags no matter what
        RemoveAppliedTags();

        // Let the BP handle the interrupt as well
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }
}

void UAgoraAbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        
        if (TagRemovalRule == EPlayMontageAndWaitForEventTagRemovalRule::Dynamic || TagRemovalRule == EPlayMontageAndWaitForEventTagRemovalRule::MontageEnding)
        {
            RemoveAppliedTags();
        }

        if (ShouldBroadcastAbilityTaskDelegates())
        {

            OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }

    EndTask();
}

void UAgoraAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        FGameplayEventData TempData = *Payload;
        TempData.EventTag = EventTag;

        if (TagRemovalRule == EPlayMontageAndWaitForEventTagRemovalRule::Dynamic || TagRemovalRule == EPlayMontageAndWaitForEventTagRemovalRule::EventBroadcast)
        {
            RemoveAppliedTags();
        }

        EventReceived.Broadcast(EventTag, TempData);
    }
}

void UAgoraAbilityTask_PlayMontageAndWaitForEvent::OnDestroy(bool AbilityEnded)
{
    // Note: Clearing montage end delegate isn't necessary since its not a multicast and will be cleared when the next montage plays.
    // (If we are destroyed, it will detect this and not do anything)

    // This delegate, however, should be cleared as it is a multicast
    if (Ability)
    {
        Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
        if (AbilityEnded && bStopWhenAbilityEnds)
        {
            StopPlayingMontage();
        }
    }

    UAgoraAbilitySystemComponent* ASC = GetTargetASC();
    if (ASC)
    {
        ASC->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
    }

    // Check for tag removal
    if (!bHasRemovedTags && bStopWhenAbilityEnds && TagRemovalRule != EPlayMontageAndWaitForEventTagRemovalRule::Custom)
    {
         TRACESTATIC(Agora, Error, "Tags %s not removed from task, despite removal rule stating they should be.", *AppliedTags.ToString());
    }

    Super::OnDestroy(AbilityEnded);
}

bool UAgoraAbilityTask_PlayMontageAndWaitForEvent::StopPlayingMontage()
{
    const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
    if (!ActorInfo)
    {
        return false;
    }

    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
    if (AnimInstance == nullptr)
    {
        return false;
    }

    // Check if the montage is still playing
    // The ability would have been interrupted, in which case we should automatically stop the montage
    if (AbilitySystemComponent && Ability)
    {
        if (AbilitySystemComponent->GetAnimatingAbility() == Ability
            && AbilitySystemComponent->GetCurrentMontage() == MontageToPlay)
        {
            // Unbind delegates so they don't get called as well
            FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
            if (MontageInstance)
            {
                MontageInstance->OnMontageBlendingOutStarted.Unbind();
                MontageInstance->OnMontageEnded.Unbind();
            }

            AbilitySystemComponent->CurrentMontageStop();

            // Need to remove tags, as the montage is ending and we're supposed to remove when montages are ending
            if (TagRemovalRule == EPlayMontageAndWaitForEventTagRemovalRule::Dynamic || TagRemovalRule == EPlayMontageAndWaitForEventTagRemovalRule::MontageEnding)
            {
                RemoveAppliedTags();
            }

            return true;
        }
    }

    return false;
}

FString UAgoraAbilityTask_PlayMontageAndWaitForEvent::GetDebugString() const
{
    UAnimMontage* PlayingMontage = nullptr;
    if (Ability)
    {
        const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
        UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

        if (AnimInstance != nullptr)
        {
            PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? MontageToPlay : AnimInstance->GetCurrentActiveMontage();
        }
    }

    return FString::Printf(TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}

void UAgoraAbilityTask_PlayMontageAndWaitForEvent::RemoveAppliedTags()
{
    if (!bHasRemovedTags)
    {
        AbilitySystemComponent->RemoveLooseGameplayTags(AppliedTags);
        bHasRemovedTags = true;
    }
}

UAgoraAbilitySystemComponent* UAgoraAbilityTask_PlayMontageAndWaitForEvent::GetTargetASC()
{
    return Cast<UAgoraAbilitySystemComponent>(AbilitySystemComponent);
}