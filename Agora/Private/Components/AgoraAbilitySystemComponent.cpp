#include "AgoraAbilitySystemComponent.h"
#include "Agora.h"
#include "UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AgoraAttributeSetBase.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraCharacterBase.h"
#include "AgoraTypes.h"
#include "AgoraDataSingleton.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "TimerManager.h"
#include "AgoraAbilityLibrary.h"
#include "AgoraAbilitySet.h"
#include "AgoraAttributeSetStatsBase.h"

void UAgoraAbilitySystemComponent::UpgradeGameplayAbility(EAbilityInput AbilitySlot)
{   
    if (!IsOwnerActorAuthoritative())
    {
        return;
    }

    if (!CanUpgradeAbility(AbilitySlot))
    {
        return;
    }

    FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromInputID((int32)AbilitySlot);
    ensure(AbilitySpec);
    if (!AbilitySpec)
    {
        return;
    }

    AbilitySpec->Level = AbilitySpec->Level + 1;
    MarkAbilitySpecDirty(*AbilitySpec);
    ForceReplication();
    Client_NotifyAbilityLeveledup(AbilitySpec->Ability->GetClass(), AbilitySpec->Level);
}

bool UAgoraAbilitySystemComponent::CanUpgradeAbility(EAbilityInput AbilityInputId)
{

    FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromInputID((int32)AbilityInputId);

    // We don't know about the ability
    if (!AbilitySpec)
    {
        return false;
    }

    int32 CurrentLevel = AbilitySpec->Level;
    int32 OwnerLevel = Cast<AAgoraCharacterBase>(GetOwner())->GetLevel();

    return ((UAgoraGameplayAbility*)AbilitySpec->Ability)->CanUpgrade(OwnerLevel, CurrentLevel + 1);
}

TSubclassOf<UAgoraGameplayAbility> UAgoraAbilitySystemComponent::GetAbilityClassFromInputID(EAbilityInput AbilitySlot)
{
    FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromInputID((int32)AbilitySlot);
    if (AbilitySpec)
    {
        return AbilitySpec->Ability->GetClass();
    }
    return nullptr;
}


const EAbilityInput* UAgoraAbilitySystemComponent::GetInputIDFromAbilityClass(TSubclassOf<UAgoraGameplayAbility> AbilityClass)
{
    FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromClass(AbilityClass);
    if(AbilitySpec)
    {
        return (EAbilityInput*)&AbilitySpec->InputID;
    }
    return nullptr;
}

FGameplayTag UAgoraAbilitySystemComponent::GetCooldownTagFromClass(TSubclassOf<UAgoraGameplayAbility> AbilityClass)

{	
	const EAbilityInput* TryFindInput = GetInputIDFromAbilityClass(AbilityClass);

	if (!TryFindInput)
	{
		return FGameplayTag::EmptyTag;
	}
	else {
		return UAgoraGameplayTags::GetAbilitySlotCooldownTag(*TryFindInput);
	}
    
    
}

bool UAgoraAbilitySystemComponent::TryActivateAbilityBySlot(EAbilityInput AbilitySlot)
{
	return TryActivateAbilityByClass(GetAbilityClassFromInputID(AbilitySlot));
}

void UAgoraAbilitySystemComponent::AddStartupGameplayAbilities()
{
	if (IsOwnerActorAuthoritative())
	{

        if(!AbilitySet) { TRACE(Agora, Warning, "No AbilitySet found for %s. No starting abilities will be provided.", *GetNameSafe(GetOwner())) }
       
        if (AbilitySet)
        {
            AbilitySet->GiveAbilities(this);
        }

      
		// Now apply passives
		SetBaseStatsFromCurveTable(1, LevelStatsCurveTable);

		for (TSubclassOf<UGameplayEffect>& GameplayEffect : PassiveStatGameplayEffects)
		{
			FGameplayEffectContextHandle EffectContext = MakeEffectContext();
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle NewHandle = MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (NewHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), this);
				PassiveStatHandles.Add(ActiveGEHandle);
			}
		}
	}
}

// Returns -1 if not found
int32 UAgoraAbilitySystemComponent::GetAbilityLevelFromClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass) { return -1; }

	FGameplayAbilitySpec* Spec = FindAbilitySpecFromClass(AbilityClass);
	return Spec ? Spec->Level : -1;
}

void UAgoraAbilitySystemComponent::SetBaseStatsFromCurveTable(int32 Level, UCurveTable* StatCurveTable)
{
	if (!ensure(!LevelsAlreadySet.Contains(Level)))
	{
		return;
	}

	TSubclassOf<UGameplayEffect> GameplayEffectClass = UAgoraBlueprintFunctionLibrary::GetGlobals()->BaseStatsAddEffect;

	if (GameplayEffectClass == nullptr || StatCurveTable == nullptr)
	{
		TRACESTATIC(Agora, Error, "%s missing GameplayEffect or CurveTable. Set these to initialise stats", *GetAvatarActor()->GetName());
		return;
	}

	// Create and fill the spec with GE Data
	FGameplayEffectSpecHandle GESpecHandle;
	FGameplayEffectContextHandle ContextHandle = FGameplayEffectContextHandle();
	GESpecHandle = MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	FGameplayEffectSpec* Spec = GESpecHandle.Data.Get();

	int32 TableIndex = Level - 1;
	const TMap<FName, FSimpleCurve*>& RowMap = StatCurveTable->GetSimpleCurveRowMap();
	// We get the StatCurveTable info to SetByCaller the right magnitudes into the Spec
	for (auto TablePair : RowMap)
	{
		FName AttrName = TablePair.Key;
		FSimpleCurve* AttrCurve = TablePair.Value;

		// Get the GameplayTag of the Attribute we're trying to change here
		FString AttributeString = AttrName.ToString();
		FString TagString = FString("SetByCaller.AttributeModifier.");
		FString CombinedString = TagString + AttributeString;
		FName DataTagName = FName(*CombinedString);

		// TODO: Turn error when not found (the boolean) back on when pen attributes are added
		FGameplayTag DataTag = FGameplayTag::RequestGameplayTag(DataTagName, false);
		
		float PreviousLevelMagnitude = 0.0f;
		if (TableIndex > 0)
		{
			FRealCurve* Curve = StatCurveTable->FindCurve(AttrName, "SetBaseStats get previous level");

			PreviousLevelMagnitude = Curve->Eval(TableIndex - 1);
		}

		// Get the magnitude for that Attribute, same Index because on same Table Row
		// Get the diff between our last level and next and add that
		float DeltaMagnitude = AttrCurve->Eval(Level) - PreviousLevelMagnitude;

		// Set the magnitude to the Tag's  corresponding GE value
		Spec->SetSetByCallerMagnitude(DataTag, DeltaMagnitude);
	}

	// Apply the GE now that the entire Spec has been filled
	ApplyGameplayEffectSpecToTarget(*Spec, this);
	LevelsAlreadySet.Add(Level);
}

void UAgoraAbilitySystemComponent::ApplySlowOnClient_Implementation(TSubclassOf<UGameplayEffect> SlowEffectClass)
{


	FGameplayEffectContextHandle EffectContext = MakeEffectContext();
	FGameplayEffectSpecHandle SlowSpecHandle = MakeOutgoingSpec(SlowEffectClass, 1, EffectContext);

	FGameplayEffectSpec* SlowSpec = SlowSpecHandle.Data.Get();
	if (SlowSpec)
	{
		ApplyGameplayEffectSpecToSelf(*SlowSpec, FPredictionKey());
	}
}

void UAgoraAbilitySystemComponent::Client_NotifyAbilityFailedLevelup_Implementation(TSubclassOf<UGameplayAbility> Ability, int32 NewLevel, EAbilityFailLevelupReason Reason)
{
    OnAbilityFailedLevelup.Broadcast(Ability, NewLevel, Reason);
}

void UAgoraAbilitySystemComponent::Client_NotifyAbilityLeveledup_Implementation(TSubclassOf<UGameplayAbility> Ability, int32 NewLevel)
{
    FGameplayAbilitySpec* Spec = FindAbilitySpecFromClass(Ability);
    if (Spec && Spec->Level != NewLevel)
    {
        Spec->Level = NewLevel;
        MarkAbilitySpecDirty(*Spec);
    }
    OnAbilityLevelup.Broadcast(Ability, NewLevel);
}

//Mostly a copypaste from engine source.
void UAgoraAbilitySystemComponent::AbilityLocalInputPressed(int32 InputID)
{
	// Consume the input if this InputID is overloaded with GenericConfirm/Cancel and the GenericConfim/Cancel callback is bound
	if (IsGenericConfirmInputBound(InputID))
	{
		LocalInputConfirm();
		return;
	}
	if (IsGenericCancelInputBound(InputID))
	{
		LocalInputCancel();
		return;
	}
	// ---------------------------------------------------------
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.InputID == InputID)
		{
			if (Spec.Ability)
			{
				Spec.InputPressed = true;
				if (Spec.IsActive())
				{
					if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
					{
						ServerSetInputPressed(Spec.Handle);
					}

					AbilitySpecInputPressed(Spec);

					// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
				}
				else
				{
					// Ability is not active, so try to activate it
					TryActivateAbility(Spec.Handle);

					//alpaca: start a timer to automatically invoke the next auto attack for as long as the LMB is held down. if the LMB is released, kill the timer.
					if (InputID == AbilityPrimaryInputID)
					{
						GetWorld()->GetTimerManager().SetTimer(BasicAttackChainingTimer, 
						[this, InputID, Spec] 
						{ 
							if (InputID == AbilityPrimaryInputID)
							{
								TryActivateAbility(Spec.Handle);
							}
							
						}, Cast<AAgoraCharacterBase>(OwnerActor)->CalculateBasicAttackTime() + 0.2f, true); //.2f buffer since the timer needs to fire just slightly after the cooldown has ended
					}
					
				}
			}
		}
	}
}

//if the LMB is released, kill the auto chaining timer
void UAgoraAbilitySystemComponent::AbilityLocalInputReleased(int32 InputID)
{
	Super::AbilityLocalInputReleased(InputID);
	if (InputID == AbilityPrimaryInputID)
	{
		GetWorld()->GetTimerManager().ClearTimer(BasicAttackChainingTimer);
	}
}
