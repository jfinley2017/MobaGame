// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraAbilityLibrary.h"
#include "AgoraAbilityEffectArea.h"
#include "AgoraGameplayAbility.h"
#include "AgoraAttributeSetStatsBase.h"
#include "AgoraDataSingleton.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffectTypes.h"
#include "Components/PrimitiveComponent.h"
#include <AbilitySystemComponent.h>

DEFINE_LOG_CATEGORY(AgoraAbility)

bool UAgoraAbilityLibrary::MatchesSweepQueryParams(FAgoraSweepQueryParams Params, AActor* SweepingActor, AActor* SweptActor)
{
	return Params.MatchesQuery(SweepingActor, SweptActor);
}

const TSet<AActor*> UAgoraAbilityLibrary::ApplyEffectInArea(UPrimitiveComponent* Area, FGameplayEffectSpecHandle Effect, FAgoraSweepQueryParams QueryParams, AActor* EffectCauser)
{
	TSet<AActor*> OverlappedActors;
	TSet<AActor*> AppliedActors;
	Area->GetOverlappingActors(OverlappedActors, APawn::StaticClass());

	for (AActor* ActorItr : OverlappedActors)
	{
		IAbilitySystemInterface* AbilityFace = Cast<IAbilitySystemInterface>(ActorItr);
		if (AbilityFace)
		{
			ensureMsgf(EffectCauser, TEXT("ApplyEffectToActor requires instigator"));
			if (QueryParams.MatchesQuery(EffectCauser, ActorItr))
			{
				UAbilitySystemComponent* ASC = AbilityFace->GetAbilitySystemComponent();
				ASC->ApplyGameplayEffectSpecToSelf(*Effect.Data, FPredictionKey());
				AppliedActors.Add(ActorItr);
			}
		}
	}

	return AppliedActors;
}

void UAgoraAbilityLibrary::GetAbilitiesWithTags(FGameplayTagContainer Tags, const UAbilitySystemComponent* ASC, TArray<UGameplayAbility*>& OutAbilities)
{
	for (FGameplayAbilitySpec Spec : ASC->GetActivatableAbilities())
	{
		UGameplayAbility* Ability = Spec.GetPrimaryInstance();
		if (Ability && Ability->AbilityTags.HasAll(Tags))
		{
			OutAbilities.Add(Ability);
		}
	}
}

TSubclassOf<UGameplayAbility> UAgoraAbilityLibrary::GetGameplayAbilityClassForInputID(AActor* Actor, EAbilityInput InputID)
{
    UAbilitySystemComponent* ActorAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
    ensure(ActorAbilitySystemComponent);

    FGameplayAbilitySpec* AbilitySpec = ActorAbilitySystemComponent->FindAbilitySpecFromInputID((int32)InputID);
    if (AbilitySpec)
    {
        return AbilitySpec->Ability->GetClass();
    }

    return nullptr;
}

int32 UAgoraAbilityLibrary::GetAbilityLevelFromInputID(AActor* Actor, EAbilityInput InputID)
{
    UAbilitySystemComponent* ActorAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
    ensure(ActorAbilitySystemComponent);

    FGameplayAbilitySpec* AbilitySpec = ActorAbilitySystemComponent->FindAbilitySpecFromInputID((int32)InputID);
    if (AbilitySpec)
    {
        return AbilitySpec->Level;
    }

    return -1;

}

bool UAgoraAbilityLibrary::GetAbilityCooldownForActor(TSubclassOf<UGameplayAbility> Ability, AActor* Actor, float& OutTimeRemaining, float& OutCooldownDuration)
{
    ensure(Ability);
    UAbilitySystemComponent* ActorAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
    ensure(ActorAbilitySystemComponent);
    

    const UGameplayAbility* AbilityCDO = Ability->GetDefaultObject<UGameplayAbility>();
    const FGameplayTagContainer* CooldownTags = AbilityCDO->GetCooldownTags();
    if (CooldownTags && CooldownTags->Num() > 0)
    {

        FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
        TArray< TPair<float, float> > DurationAndTimeRemaining = ActorAbilitySystemComponent->GetActiveEffectsTimeRemainingAndDuration(Query);
        if (DurationAndTimeRemaining.Num() > 0)
        {
            int32 BestIdx = 0;
            float LongestTime = DurationAndTimeRemaining[0].Key;
            for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
            {
                if (DurationAndTimeRemaining[Idx].Key > LongestTime)
                {
                    LongestTime = DurationAndTimeRemaining[Idx].Key;
                    BestIdx = Idx;
                }
            }

            OutTimeRemaining = DurationAndTimeRemaining[BestIdx].Key;
            OutCooldownDuration = DurationAndTimeRemaining[BestIdx].Value;
            return true;
        }
    }

    return false;
}

bool UAgoraAbilityLibrary::GetAbilityCost(TSubclassOf<UGameplayAbility> Ability, AActor* Actor, float& Cost, FGameplayAttribute& Attribute)
{
    if (!Ability)
    {
        TRACESTATIC(AgoraAbility,Warning, "Null ability. Could not determine cost.")
        return false;
    }
    
    UAbilitySystemComponent* ActorAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
    if (!ActorAbilitySystemComponent)
    {
        TRACESTATIC(AgoraAbility, Warning, "Actor did not have an AbilitySystemComponent. Could not determine cost of ability %s", *GetNameSafe(Ability));
        return false;
    }

    FGameplayAbilitySpec* AbilitySpec = ActorAbilitySystemComponent->FindAbilitySpecFromClass(Ability);
    if (!AbilitySpec)
    {
        // If we don't have an ability spec (we haven't been given the ability), return false
        TRACESTATIC(AgoraAbility, Warning, "Ability %s not found for actor %s. Could not determine cost.", *GetNameSafe(Ability), *GetNameSafe(Actor));
        return false;
    }
    
	// Primary instance is required, otherwise it will always be level 1
    UAgoraGameplayAbility* AgAbility = Cast<UAgoraGameplayAbility>(AbilitySpec->GetPrimaryInstance());
    Cost = AgAbility->GetScaledCostMagnitude();
    Attribute = AgAbility->GetCostAttribute();

    return true;

}

bool UAgoraAbilityLibrary::CanAbilityBeLeveled(AActor* Actor, TSubclassOf<UGameplayAbility> Ability)
{
    if (!Ability)
    {
        TRACESTATIC(AgoraAbility, Warning, "Null ability. Could not determine whether it could be leveled.")
        return false;
    }

    UAbilitySystemComponent* ActorAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
    if (!ActorAbilitySystemComponent)
    {
        TRACESTATIC(AgoraAbility, Warning, "No ability system found for %s, could not determine if ability %s could be leveled.", *GetNameSafe(Ability), *GetNameSafe(Actor));
        return false;
    }

    // interface plz, not converting from because this needs to be changed eventually 
    AAgoraCharacterBase* ActorAsCharacter = Cast<AAgoraCharacterBase>(Actor);
    ensure(ActorAsCharacter);

    FGameplayAbilitySpec* AbilitySpec = ActorAbilitySystemComponent->FindAbilitySpecFromClass(Ability);
    if (!AbilitySpec)
    {
        return false;
    }

    return ((UAgoraGameplayAbility*)AbilitySpec->Ability)->CanUpgrade(ActorAsCharacter->GetLevel(), AbilitySpec->Level+1);

}

int32 UAgoraAbilityLibrary::GetAbilityLevelFromClass(AActor* Actor, TSubclassOf<UGameplayAbility> Ability)
{
    ensure(Ability);
    UAbilitySystemComponent* ActorAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
    ensure(ActorAbilitySystemComponent);

    FGameplayAbilitySpec* AbilitySpec = ActorAbilitySystemComponent->FindAbilitySpecFromClass(Ability);
    if (!AbilitySpec)
    {
        return -1;
    }

    return AbilitySpec->Level;
}

int32 UAgoraAbilityLibrary::GetAbilityMaxLevelFromClass(TSubclassOf<UGameplayAbility> Ability)
{
    ensure(Ability);

    return Ability->GetDefaultObject<UAgoraGameplayAbility>()->GetMaxLevel();

}

void UAgoraAbilityLibrary::AddGameplayCueToASC(UAbilitySystemComponent* ASC, const FGameplayTag GameplayCueTag)
{
	ASC->AddGameplayCue(GameplayCueTag, FGameplayEffectContextHandle());
}

void UAgoraAbilityLibrary::ExecuteGameplayCueOnASC(UAbilitySystemComponent* ASC, const FGameplayTag GameplayCueTag)
{
	ASC->ExecuteGameplayCue(GameplayCueTag, FGameplayEffectContextHandle());
}

FAgoraAbilityMetadata UAgoraAbilityLibrary::GetAbilityMetadata(TSubclassOf<UGameplayAbility> AbilityClass)
{
	TSubclassOf<UAgoraGameplayAbility> AgAbility = TSubclassOf<UAgoraGameplayAbility>(*AbilityClass);
	if (AgAbility)
	{
		return AgAbility->GetDefaultObject<UAgoraGameplayAbility>()->GetAbilityMetadata();
	}
	
	return FAgoraAbilityMetadata();
}

FText UAgoraAbilityLibrary::GetAbilityDescription(TSubclassOf<UAgoraGameplayAbility> Ability, int32 Level)
{
    if (!Ability)
    {
        return FText::FromString("None");
    }
	return  Ability->GetDefaultObject<UAgoraGameplayAbility>()->GetAbilityDescription(Level);
}


TSubclassOf<UAgoraGameplayAbility> UAgoraAbilityLibrary::GetAbilityClassByInputId(AActor* Actor, EAbilityInput InputId)
{
    UAgoraAbilitySystemComponent* AgoraASC 
        = Cast<UAgoraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor));

    if (AgoraASC)
    {
        return AgoraASC->GetAbilityClassFromInputID(InputId);
    }
    return nullptr;
}

float UAgoraAbilityLibrary::GetFloatAttributeChecked(const AActor* Actor, FGameplayAttribute Attribute)
{
	bool bSuccessful = true;
	float Val = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Actor, Attribute, bSuccessful);

	if (bSuccessful)
	{
		return Val;
	}

	TRACESTATIC(AgoraAbility, Fatal, "Attribute %s not returned from %s, does it have the attribute you are trying to access?", *GetNameSafe(Actor), *Attribute.GetName());
	return 0.0f; // won't hit
}

void UAgoraAbilityLibrary::AddLooseGameplayTagToASC(const FGameplayTag GameplayTag, UAbilitySystemComponent* AbilitySystemComponent, int32 Count = 1)
{
	if (!AbilitySystemComponent)
	{
		TRACESTATIC(AgoraAbility, Warning, "AddLooseGameplayTag %s called, but invalid AbilitySystemComponent provided (was null).", *GameplayTag.ToString())
        return;
	}

	AbilitySystemComponent->AddLooseGameplayTag(GameplayTag, Count);
}

void UAgoraAbilityLibrary::RemoveLooseGameplayTagFromASC(const FGameplayTag GameplayTag, UAbilitySystemComponent* AbilitySystemComponent, int32 Count = 1)
{
	if (!AbilitySystemComponent)
	{
		TRACESTATIC(AgoraAbility, Warning, "AddLooseGameplayTag %s called, but invalid AbilitySystemComponent provided (was null).", *GameplayTag.ToString())
        return;
	}

	AbilitySystemComponent->RemoveLooseGameplayTag(GameplayTag, Count);
}

const FGameplayTagContainer UAgoraAbilityLibrary::BP_GetCooldownTags(TSubclassOf<UGameplayAbility> Ability)
{
	const FGameplayTagContainer* Tags = GetCooldownTags(Ability);
	
	return Tags ? *Tags : FGameplayTagContainer();
}

const FGameplayTagContainer* UAgoraAbilityLibrary::GetCooldownTags(TSubclassOf<UGameplayAbility> Ability)
{
	return Ability->GetDefaultObject<UGameplayAbility>()->GetCooldownTags();
}

FGameplayEffectSpecHandle UAgoraAbilityLibrary::SetEffectDamageType(FGameplayEffectSpecHandle Spec, EDamageType DamageType/*= EDamageType::DMGInvalid*/)
{
	switch (DamageType) {
	case EDamageType::DMGInvalid:
		break;
	case EDamageType::DMGMagical:
		UAbilitySystemBlueprintLibrary::AddAssetTag(Spec, UAgoraGameplayTags::DamageTypeMagical());
		break;
	case EDamageType::DMGPhysical:
		UAbilitySystemBlueprintLibrary::AddAssetTag(Spec, UAgoraGameplayTags::DamageTypePhysical());
		break;
	case EDamageType::DMGTrue:
		break;
	default:
		break;
	}

	return Spec;
}

FGameplayEffectSpecHandle UAgoraAbilityLibrary::GetDamageSpec(FGameplayEffectContextHandle Context, float Damage /*= 100.0*/, float ScalingValue /*= 1.0*/, EDamageType DamageType/*= EDamageType::DMGInvalid*/)
{
	TSubclassOf<UGameplayEffect> DamageEffect = UAgoraBlueprintFunctionLibrary::GetGlobals()->SetByCallerDamageEffect;
	
	UGameplayEffect* GameplayEffect = DamageEffect->GetDefaultObject<UGameplayEffect>();
	
	if (!Context.IsValid()) 
	{
		TRACESTATIC(AgoraAbility, Error, "UAgoraAbilityLibrary::GetDamageSpec was supplied an invalid context");
	}

	// Level is irrelevant for set by caller effects
	FGameplayEffectSpec* Spec = new FGameplayEffectSpec(GameplayEffect, Context, 1);
	FGameplayEffectSpecHandle SpecHandle = FGameplayEffectSpecHandle(Spec);
	SetDamageSpecValues(SpecHandle, Damage, ScalingValue, DamageType);

	return SpecHandle;
}

FGameplayEffectSpecHandle UAgoraAbilityLibrary::GetDamageSpec(UAbilitySystemComponent* AbilitySystem, float Damage /*= 100.0*/, float ScalingValue /*= 1.0*/, EDamageType DamageType /*= EDamageType::DMGInvalid*/)
{
	if (!ensureMsgf(AbilitySystem, TEXT("AbilitySystem must be specified")))
	{
		return FGameplayEffectSpecHandle();
	}

	return GetDamageSpec(AbilitySystem->MakeEffectContext(), Damage, ScalingValue, DamageType);
}

bool UAgoraAbilityLibrary::FindAbilitySpecHandleBySlot(EAbilityInput AbilitySlot, UAbilitySystemComponent* ASC, FGameplayAbilitySpecHandle& OutAbilityHandle)
{
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromInputID(static_cast<int32>(AbilitySlot));

	if (!Spec)
	{
		return false;
	}
	else
	{
		OutAbilityHandle = Spec->Handle;
		return true;
	}
}

void UAgoraAbilityLibrary::GetAttributeName(FGameplayAttribute Attribute, FString& OutAttributeName)
{
    OutAttributeName = Attribute.GetName();
}

FGameplayEffectSpecHandle UAgoraAbilityLibrary::SetDamageSpecValues(FGameplayEffectSpecHandle SpecHandle, float Damage /*= 100.0*/, float ScalingValue /*= 1.0*/, EDamageType DamageType /*= EDamageType::DMGInvalid*/)
{
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle,
		FGameplayTag::RequestGameplayTag("SetByCaller.BaseDamage"), Damage);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle,
		FGameplayTag::RequestGameplayTag("SetByCaller.ScalingValue"), ScalingValue);

	SetEffectDamageType(SpecHandle, DamageType);

	return SpecHandle;
}

//////////////////////////////////////////////////////////////////////////
// UAgoraAbilitySet
//////////////////////////////////////////////////////////////////////////

