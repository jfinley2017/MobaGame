// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.


#include "AgoraAbilityTask_WaitTargetData.h"
#include "AbilitySystemComponent.h" // ASC
#include "Engine/Engine.h"
#include "AIController.h"


UAgoraAbilityTask_WaitTargetData* UAgoraAbilityTask_WaitTargetData::AgoraWaitTargetData(UGameplayAbility* OwningAbility, FName TaskInstanceName, FGameplayTag GameplayCueTag, FGameplayCueParameters GameplayCueParameters, FGameplayTagContainer TagsToApplyToOwner, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, TSubclassOf<AGameplayAbilityTargetActor> Class)
{
    UAgoraAbilityTask_WaitTargetData* MyObj = NewAbilityTask<UAgoraAbilityTask_WaitTargetData>(OwningAbility, TaskInstanceName);        //Register for task list here, providing a given FName as a key
    MyObj->GameplayCueTag = GameplayCueTag;
    MyObj->GameplayCueParameters = GameplayCueParameters; 
    MyObj->TagsToApplyToOwner = TagsToApplyToOwner;
    MyObj->TargetClass = Class;
    MyObj->TargetActor = nullptr;
    MyObj->ConfirmationType = ConfirmationType;
    return MyObj;
}

UAgoraAbilityTask_WaitTargetData* UAgoraAbilityTask_WaitTargetData::AgoraWaitTargetDataUsingActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, FGameplayTag GameplayCueTag, FGameplayCueParameters GameplayCueParameters, FGameplayTagContainer TagsToApplyToOwner, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor)
{
    UAgoraAbilityTask_WaitTargetData* MyObj = NewAbilityTask<UAgoraAbilityTask_WaitTargetData>(OwningAbility, TaskInstanceName);		//Register for task list here, providing a given FName as a key
    MyObj->GameplayCueTag = GameplayCueTag;
    MyObj->GameplayCueParameters = GameplayCueParameters;
    MyObj->TagsToApplyToOwner = TagsToApplyToOwner;
    MyObj->TargetClass = nullptr;
    MyObj->TargetActor = InTargetActor;
    MyObj->ConfirmationType = ConfirmationType;
    return MyObj;
}

// This method is not executed if the Class actor is passed in as a spawn param. This appears to be done through some UHT magic
void UAgoraAbilityTask_WaitTargetData::Activate()
{
    // Need to handle case where target actor was passed into task
    if (Ability && (TargetClass == nullptr))
    {
        if (TargetActor)
        {
            AGameplayAbilityTargetActor* SpawnedActor = TargetActor;
            TargetClass = SpawnedActor->GetClass();

            RegisterTargetDataCallbacks();
            AbilitySystemComponent->AddLooseGameplayTags(TagsToApplyToOwner);
            AbilitySystemComponent->AddGameplayCue(GameplayCueTag, GameplayCueParameters);

            if (IsPendingKill())
            {
                return;
            }

			if (ShouldSpawnTargetActor())
            {
                InitializeTargetActor(SpawnedActor);
                FinalizeTargetActor(SpawnedActor);

                // Note that the call to FinalizeTargetActor, this task could finish and our owning ability may be ended.
            }
            else
            {
                TargetActor = nullptr;

                // We may need a better solution here.  We don't know the target actor isn't needed till after it's already been spawned.
                SpawnedActor->Destroy();
                SpawnedActor = nullptr;
            }
        }
        else
        {
            EndTask();
        }
    }
}

bool UAgoraAbilityTask_WaitTargetData::BeginSpawningActor(UGameplayAbility* OwningAbility, TSubclassOf<AGameplayAbilityTargetActor> InTargetClass, AGameplayAbilityTargetActor*& SpawnedActor)
{
    SpawnedActor = nullptr;

    if (Ability)
    {
		if (ShouldSpawnTargetActor())
        {
            UClass* Class = *InTargetClass;
            if (Class != nullptr)
            {
                if (UWorld* World = GEngine->GetWorldFromContextObject(OwningAbility, EGetWorldErrorMode::LogAndReturnNull))
                {
                    SpawnedActor = World->SpawnActorDeferred<AGameplayAbilityTargetActor>(Class, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
                }
            }

            if (SpawnedActor)
            {
                TargetActor = SpawnedActor;
                InitializeTargetActor(SpawnedActor);
            }
        }

        RegisterTargetDataCallbacks();
        AbilitySystemComponent->AddLooseGameplayTags(TagsToApplyToOwner);
        AbilitySystemComponent->AddGameplayCue(GameplayCueTag, GameplayCueParameters);

    }

    return (SpawnedActor != nullptr);
}

void UAgoraAbilityTask_WaitTargetData::FinishSpawningActor(UGameplayAbility* OwningAbility, AGameplayAbilityTargetActor* SpawnedActor)
{
    if (SpawnedActor && !SpawnedActor->IsPendingKill())
    {
        check(TargetActor == SpawnedActor);

        const FTransform SpawnTransform = AbilitySystemComponent->GetOwner()->GetTransform();

        SpawnedActor->FinishSpawning(SpawnTransform);

        FinalizeTargetActor(SpawnedActor);
    }
}

bool UAgoraAbilityTask_WaitTargetData::ShouldSpawnTargetActor() const
{
    check(TargetClass);
    check(Ability);

    // Spawn the actor if this is a locally controlled ability (always) or if this is a replicating targeting mode.
    // (E.g., server will spawn this target actor to replicate to all non owning clients)

    const AGameplayAbilityTargetActor* CDO = CastChecked<AGameplayAbilityTargetActor>(TargetClass->GetDefaultObject());

    const bool bReplicates = CDO->GetIsReplicated();
    const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
    const bool bShouldProduceTargetDataOnServer = CDO->ShouldProduceTargetDataOnServer;

    return (bReplicates || bIsLocallyControlled || bShouldProduceTargetDataOnServer);
}

void UAgoraAbilityTask_WaitTargetData::InitializeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const
{
    check(SpawnedActor);
    check(Ability);

    SpawnedActor->MasterPC = Ability->GetCurrentActorInfo()->PlayerController.Get();

    // If we spawned the target actor, always register the callbacks for when the data is ready.
    SpawnedActor->TargetDataReadyDelegate.AddUObject(this, &UAgoraAbilityTask_WaitTargetData::OnTargetDataReadyCallback);
    SpawnedActor->CanceledDelegate.AddUObject(this, &UAgoraAbilityTask_WaitTargetData::OnTargetDataCancelledCallback);
}

void UAgoraAbilityTask_WaitTargetData::FinalizeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const
{
    check(SpawnedActor);
    check(Ability);

    // User ability activation is inhibited while this is active
    AbilitySystemComponent->SpawnedTargetActors.Push(SpawnedActor);

    SpawnedActor->StartTargeting(Ability);

	if (IsAIControlled())
	{
		SpawnedActor->ConfirmTargeting();
	}
    else if (SpawnedActor->ShouldProduceTargetData())
    {
        // If instant confirm, then stop targeting immediately.
        // Note this is kind of bad: we should be able to just call a static func on the CDO to do this. 
        // But then we wouldn't get to set ExposeOnSpawnParameters.
        if (ConfirmationType == EGameplayTargetingConfirmation::Instant)
        {
            SpawnedActor->ConfirmTargeting();
        }
        else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
        {
            // Bind to the Cancel/Confirm Delegates (called from local confirm or from repped confirm)
            SpawnedActor->BindToConfirmCancelInputs();
        }
    }
}

void UAgoraAbilityTask_WaitTargetData::RegisterTargetDataCallbacks()
{
    if (!ensure(IsPendingKill() == false))
    {
        return;
    }

    check(TargetClass);
    check(Ability);

    const AGameplayAbilityTargetActor* CDO = CastChecked<AGameplayAbilityTargetActor>(TargetClass->GetDefaultObject());

    const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
    const bool bShouldProduceTargetDataOnServer = CDO->ShouldProduceTargetDataOnServer;

    // If not locally controlled (server for remote client), see if TargetData was already sent
    // else register callback for when it does get here.
    if (!bIsLocallyControlled)
    {
        // Register with the TargetData callbacks if we are expecting client to send them
        if (!bShouldProduceTargetDataOnServer)
        {
            FGameplayAbilitySpecHandle	SpecHandle = GetAbilitySpecHandle();
            FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

            //Since multifire is supported, we still need to hook up the callbacks
            AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UAgoraAbilityTask_WaitTargetData::OnTargetDataReplicatedCallback);
            AbilitySystemComponent->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UAgoraAbilityTask_WaitTargetData::OnTargetDataReplicatedCancelledCallback);

            AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

            SetWaitingOnRemotePlayerData();
        }
    }
}

/** Valid TargetData was replicated to use (we are server, was sent from client) */
void UAgoraAbilityTask_WaitTargetData::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)
{
    check(AbilitySystemComponent);

    FGameplayAbilityTargetDataHandle MutableData = Data;
    AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

    /**
     *  Call into the TargetActor to sanitize/verify the data. If this returns false, we are rejecting
     *	the replicated target data and will treat this as a cancel.
     *
     *	This can also be used for bandwidth optimizations. OnReplicatedTargetDataReceived could do an actual
     *	trace/check/whatever server side and use that data. So rather than having the client send that data
     *	explicitly, the client is basically just sending a 'confirm' and the server is now going to do the work
     *	in OnReplicatedTargetDataReceived.
     */
    if (TargetActor && !TargetActor->OnReplicatedTargetDataReceived(MutableData))
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            Cancelled.Broadcast(MutableData);
        }
    }
    else
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            ValidData.Broadcast(MutableData);
        }
    }

    if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
    {
        EndTask();
    }
}

/** Client canceled this Targeting Task (we are the server) */
void UAgoraAbilityTask_WaitTargetData::OnTargetDataReplicatedCancelledCallback()
{
    check(AbilitySystemComponent);
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
    }
    EndTask();
}

/** The TargetActor we spawned locally has called back with valid target data */
void UAgoraAbilityTask_WaitTargetData::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data)
{
    check(AbilitySystemComponent);
    if (!Ability)
    {
        return;
    }

    FScopedPredictionWindow	ScopedPrediction(AbilitySystemComponent, ShouldReplicateDataToServer());

    const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
    if (IsPredictingClient())
    {
        if (!TargetActor->ShouldProduceTargetDataOnServer)
        {
            FGameplayTag ApplicationTag; // Fixme: where would this be useful?
            AbilitySystemComponent->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(), Data, ApplicationTag, AbilitySystemComponent->ScopedPredictionKey);
        }
        else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
        {
            // We aren't going to send the target data, but we will send a generic confirmed message.
            AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
        }
    }

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        ValidData.Broadcast(Data);
    }

    if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
    {
        EndTask();
    }
}

/** The TargetActor we spawned locally has called back with a cancel event (they still include the 'last/best' targetdata but the consumer of this may want to discard it) */
void UAgoraAbilityTask_WaitTargetData::OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data)
{
    check(AbilitySystemComponent);

    FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent, IsPredictingClient());

    if (IsPredictingClient())
    {
        if (!TargetActor->ShouldProduceTargetDataOnServer)
        {
            AbilitySystemComponent->ServerSetReplicatedTargetDataCancelled(GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
        }
        else
        {
            // We aren't going to send the target data, but we will send a generic confirmed message.
            AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
        }
    }
    Cancelled.Broadcast(Data);
    EndTask();
}

/** Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true. */
void UAgoraAbilityTask_WaitTargetData::ExternalConfirm(bool bEndTask)
{
    check(AbilitySystemComponent);
    if (TargetActor)
    {
        if (TargetActor->ShouldProduceTargetData())
        {
            TargetActor->ConfirmTargetingAndContinue();
        }
    }
    Super::ExternalConfirm(bEndTask);
}

/** Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true. */
void UAgoraAbilityTask_WaitTargetData::ExternalCancel()
{
    check(AbilitySystemComponent);
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
    }

    Super::ExternalCancel();
}

void UAgoraAbilityTask_WaitTargetData::OnDestroy(bool AbilityEnded)
{
    if (TargetActor)
    {
        TargetActor->Destroy();
    }

    AbilitySystemComponent->RemoveLooseGameplayTags(TagsToApplyToOwner);
    AbilitySystemComponent->RemoveGameplayCue(GameplayCueTag);

    Super::OnDestroy(AbilityEnded);
}

bool UAgoraAbilityTask_WaitTargetData::IsAIControlled() const
{
	APawn* AbilityPawn = Cast<APawn>(Ability->GetActorInfo().OwnerActor);
	if (ensureMsgf(AbilityPawn, TEXT("WaitTargetData relies on the ability owner actor being a pawn, if its not please extend this")))
	{
		if (Cast<AAIController>(AbilityPawn->GetController()))
		{
			return true;
		}
		if (!AbilityPawn->GetPlayerState())
		{
			return true;
		}
		if (AbilityPawn->GetController())
		{
			return false;
		}

		return false;
	}

	return false;
}

bool UAgoraAbilityTask_WaitTargetData::ShouldReplicateDataToServer() const
{
    if (!Ability || !TargetActor)
    {
        return false;
    }

    // Send TargetData to the server IFF we are the client and this isn't a GameplayTargetActor that can produce data on the server	
    const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
    if (!Info->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
    {
        return true;
    }

    return false;
}


// --------------------------------------------------------------------------------------
