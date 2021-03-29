#include "AgoraGameplayAbility.h"
#include "Engine/Engine.h"
#include "Misc/ConfigCacheIni.h"
#include "Camera/CameraComponent.h"

#include "Pawns/AgoraCharacterBase.h"
#include "Components/AgoraLevelComponent.h"
#include "AgoraDamageExecutionCalculation.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Agora.h"

#include "AgoraTypes.h" // GameplayTags
#include "AbilitySystemComponent.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraDataSingleton.h"
#include "AgoraHeroBase.h"
#include "AgoraAbilityLibrary.h"
#include "AgoraGameplayTags.h"
#include "AgoraAttributeSetStatsBase.h"
#include "CooldownGameplayEffect.h"
#include <ConstructorHelpers.h>
#include "AgoraGameplayTypes.h"

UAgoraGameplayAbility::UAgoraGameplayAbility()
{
    // Default Ability settings
    bServerRespectsRemoteAbilityCancellation = false;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    GlobalCooldownDuration = 0.5f;

	// We have a separate CooldownGEClass to make it easier to filter out non-cooldown GE's, it only looks for GE's subclassed from CooldownGameplayEffect
	// The default is also provided via the class finder
	if (!CooldownGEClass)
	{
		ConstructorHelpers::FClassFinder<UCooldownGameplayEffect> DefaultCooldownFinder(TEXT("/Game/Blueprints/Abilities/Cooldowns/GE_AbilityCooldown"));
		CooldownGEClass = DefaultCooldownFinder.Class;
	}
	ConstructorHelpers::FObjectFinder<UDataTable> DefaultAbilityMetadataTable(TEXT("/Game/Blueprints/Heroes/DT_AbilityMetadata"));
	AbilityMetadataTable = DefaultAbilityMetadataTable.Object;

	if (bUseCharges)
	{
		TagCountContainer.SetTagCount(ChargeTag, 0);
	}
}

bool UAgoraGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /* = nullptr */, const FGameplayTagContainer* TargetTags /* = nullptr */, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */) const
{
    if (GetAbilityLevel() == 0)
    {
        return false;
    }

	if (bUseCharges && GetChargeCount() >= MaxCharges)
	{
		return false;
	}

    return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

bool UAgoraGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags /* = nullptr */, const FGameplayTagContainer* TargetTags /* = nullptr */, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */) const
{

    // Some abilities don't want to be used if the owner is currently targeting
    if (ActivationTargetingRules == EActivationTargetingRules::NotActivatableWhileTargeting && AbilitySystemComponent.HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Skills.Targeting")))
    {
        return false;
    }

    // Some abilities don't want to be used if the owner is currently involved in a task 
    if (!bActivatableWhileCasting && AbilitySystemComponent.HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Skills.Casting")))
    {
        return false;
    }

    if (!bActivatableWhileDead && AbilitySystemComponent.HasMatchingGameplayTag(UAgoraGameplayTags::Dead()))
    {
        return false;
    }

    return Super::DoesAbilitySatisfyTagRequirements(AbilitySystemComponent, SourceTags, TargetTags, OptionalRelevantTags);
}

void UAgoraGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CostAttribute.IsValid())
	{
		// this is okay - some abilities don't have a cost
		return;
	}

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
    check(AbilitySystemComponent);
    AbilitySystemComponent->ApplyGameplayEffectToSelf(GetCostGameplayEffect(), GetAbilityLevel(), AbilitySystemComponent->MakeEffectContext());
}

UGameplayEffect* UAgoraGameplayAbility::GetCostGameplayEffect() const
{
    UGameplayEffect* CostEffect = nullptr;

	// Handle non-standard cases with a custom GE, or turn this into a custom calculation class that can check things like
	// which items you have, hero passives, and implement some standard interface that can be called to modify this
    if (CostGameplayEffectClass || !CostAttribute.IsValid())
    {
		return Super::GetCostGameplayEffect();
    }
    else
    {
		// maybe make this a class instead so we can instance it
        CostEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("CostEffect")));
        FGameplayModifierInfo Info;
        Info.ModifierMagnitude = GetAbilityStatCurve(CostRowName);
        Info.ModifierOp = EGameplayModOp::Additive;
        Info.Attribute = CostAttribute;
        CostEffect->Modifiers.Add(Info);
        CostEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
    }
    
    return CostEffect;
}

void UAgoraGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	ApplyGameplayEffectToOwnerWithTags(GetCooldownTags(), Handle, ActorInfo, ActivationInfo, CooldownGEClass);
}

FAgoraAbilityMetadata UAgoraGameplayAbility::GetAbilityMetadata()
{
	if (!AbilityMetadataTable)
	{
		return FAgoraAbilityMetadata();
	}

	FAgoraAbilityMetadata* AbilityData = AbilityMetadataTable->FindRow<FAgoraAbilityMetadata>(AbilityNameId, "GetAbilityMetadata");
	if (AbilityData)
	{
		return *AbilityData;
	}
	return FAgoraAbilityMetadata();
}

FText UAgoraGameplayAbility::GetAbilityDescription(int32 Level)
{
	FText Description = GetAbilityMetadata().Description;
	FFormatNamedArguments NamedArguments;
	
	for (TPair<FName, FRealCurve*> Row : GetAbilityLevelTable()->GetRowMap())
	{
		FScalableFloat StatLevels = GetAbilityStatCurve(Row.Key);
		NamedArguments.Add(Row.Key.ToString(), StatLevels.GetValueAtLevel(Level));
	}

	FText FormattedText = FText::Format(Description, NamedArguments);

	FormatDescriptionText(Level, FormattedText, FormattedText);

	return FormattedText;
}

APawn* UAgoraGameplayAbility::GetOwningActorFromActorInfoAsPawn() const
{
	APawn* OwningActorAsPawn = Cast<APawn>(GetOwningActorFromActorInfo());
    return OwningActorAsPawn;
}

AAgoraCharacterBase* UAgoraGameplayAbility::GetOwningActorFromActorInfoAsAgoraCharacter() const
{
	AAgoraCharacterBase* Character = Cast<AAgoraCharacterBase>(GetOwningActorFromActorInfo());
	return Character;
}

UGASCharacterMovementComponent* UAgoraGameplayAbility::GetOwningActorGASCMC() const
{
	AAgoraCharacterBase* Character = Cast<AAgoraCharacterBase>(GetOwningActorFromActorInfo());
	if (Character)
	{
		return Character->GetGASCMC();
	}
	else
	{
		TRACESTATIC(Agora, Fatal, "%s tried to be an AgoraCharacter with a GASCMC. They failed.", *GetNameSafe(GetOwningActorFromActorInfo()));
	}
	return NULL;
}

bool UAgoraGameplayAbility::CanUpgrade(int32 HeroLevel, int32 SpecAbilityLevel)
{
	if (!AbilityLevelTable) { return false; }

	FSimpleCurve* Curve = AbilityLevelTable->FindSimpleCurve("heroLevelRequired", "");

	int32 HeroLevelRequired = static_cast<int32>(Curve->Eval(SpecAbilityLevel, -1));

	if (HeroLevelRequired == -1) { 
		return false; 
	}

	return HeroLevelRequired <= HeroLevel;
}

UCameraComponent* UAgoraGameplayAbility::GetCameraComponentFromOwningActor() const
{
    AAgoraHeroBase* OwnerAsHero = Cast<AAgoraHeroBase>(GetOwningActorFromActorInfo());
    if (OwnerAsHero)
    {
        return OwnerAsHero->GetCameraComponent();
    }
    return nullptr;
}

void UAgoraGameplayAbility::MoveCameraTo(FVector TargetRelativeLocation, FRotator TargetRelativeRotation, bool bEaseOut, bool bEaseIn, float OverTime, FLatentActionInfo LatentInfo)
{
	UCameraComponent* CameraComponent = GetCameraComponentFromOwningActor();
	TEnumAsByte<EMoveComponentAction::Type> MoveAction = EMoveComponentAction::Move;
	UKismetSystemLibrary::MoveComponentTo(CameraComponent, TargetRelativeLocation, TargetRelativeRotation, bEaseOut, bEaseIn, OverTime, false, MoveAction, LatentInfo);
}

void UAgoraGameplayAbility::ResetCamera(bool bEaseOut, bool bEaseIn, float OverTime, FLatentActionInfo LatentInfo)
{
	UCameraComponent* CameraComponent = GetCameraComponentFromOwningActor();
	TEnumAsByte<EMoveComponentAction::Type> MoveAction = EMoveComponentAction::Move;
	UKismetSystemLibrary::MoveComponentTo(CameraComponent, DefaultHeroCameraRelativeLocation, DefaultHeroCameraRelativeRotation, bEaseOut, bEaseIn, OverTime, false, MoveAction, LatentInfo);
}

float UAgoraGameplayAbility::GetScaledCostMagnitude() const
{
	if (CostAttribute.IsValid())
	{
		return GetAbilityStat(CostRowName);
	}

	return 0;
}

FGameplayAttribute UAgoraGameplayAbility::GetCostAttribute() const
{
	return CostAttribute;
}

int32 UAgoraGameplayAbility::GetMaxLevel() const
{
    ensure(AbilityLevelTable);
    return AbilityLevelTable->FindSimpleCurve("heroLevelRequired", "")->GetNumKeys();
}

UCurveTable* UAgoraGameplayAbility::GetAbilityLevelTable() const
{
	return AbilityLevelTable;
}

FGameplayEffectContextHandle UAgoraGameplayAbility::MakeInstancedEffectContext()
{
	return MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
}

EAbilityInput UAgoraGameplayAbility::GetAbilitySlot() const
{
	return static_cast<EAbilityInput>(GetAbilitySystemComponentFromActorInfo()->FindAbilitySpecFromClass(StaticClass())->InputID);
}

void UAgoraGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // Log that the ability was activated
    //LogAbility(FString("Ability Activated."), EAbilityLogType::AgoraAbility, false);

    if (ActivationTargetingRules == EActivationTargetingRules::ActivatableWhileTargetingExclusive)
    {
        // Destroy any ongoing targeting actions if we are activated.
        GetAbilitySystemComponentFromActorInfo()->TargetCancel();
    }

    UCameraComponent* CameraComponent = GetCameraComponentFromOwningActor();
	if (CameraComponent)
	{
		DefaultHeroCameraRelativeLocation = CameraComponent->RelativeLocation;
		DefaultHeroCameraRelativeRotation = CameraComponent->RelativeRotation;
	}


    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAgoraGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // Log that the ability ended
    //LogAbility(FString("Ability Ended."), EAbilityLogType::AgoraAbility, false);
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAgoraGameplayAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::CommitExecute(Handle, ActorInfo, ActivationInfo);

    if (bIncursGlobalCooldown)
    {
        UGameplayEffect* GlobalCooldownEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GlobalCooldownEffect")));
        GlobalCooldownEffect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
		FGameplayEffectModifierMagnitude EffectDuration = FScalableFloat(GlobalCooldownDuration);
        GlobalCooldownEffect->DurationMagnitude = EffectDuration;
        FGameplayTag RequestedTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.Global"));
        GlobalCooldownEffect->InheritableOwnedTagsContainer.AddTag(RequestedTag);
        GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectToSelf(GlobalCooldownEffect, 1, GetAbilitySystemComponentFromActorInfo()->MakeEffectContext());
    }

	if (bUseCharges)
	{
		TagCountContainer.SetTagCount(ChargeTag, GetChargeCount() + 1);
		check(GetChargeCount() <= MaxCharges); // This shouldn't be possible, because can activate ability checks charge count
	}
}

int32 UAgoraGameplayAbility::GetChargeCount() const
{
	return TagCountContainer.GetTagCount(ChargeTag);
}

void UAgoraGameplayAbility::RefreshCharges(int32 NumCharges /*= -1*/)
{
	if (NumCharges < 0)
	{
		NumCharges = 0;
	}
	else
	{
		NumCharges = GetChargeCount() - NumCharges;
		NumCharges = NumCharges > 0 ? NumCharges : 0;
	}

	TagCountContainer.SetTagCount(ChargeTag, NumCharges);
}

FActiveGameplayEffectHandle UAgoraGameplayAbility::ApplyGameplayEffectToOwnerWithTags(const FGameplayTagContainer* DynamicGrantedTags, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, TSubclassOf<UGameplayEffect> GameplayEffectClass, int32 Stacks /*= 1*/) const
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) && GameplayEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, GameplayEffectClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->StackCount = Stacks;
			
			if (DynamicGrantedTags)
			{
				SpecHandle.Data->DynamicGrantedTags = *DynamicGrantedTags;
			}

			return ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}

	return FActiveGameplayEffectHandle();
}

FGameplayEffectSpecHandle UAgoraGameplayAbility::SetScaledDamageSpec(FGameplayEffectSpecHandle SpecHandle, EDamageType DamageType /*= EDamageType::DMGInvalid*/, FName CoefficientRow /*= FName("DamageCoefficient")*/, FName BaseDamageRow /*= "damage"*/, float Multiplier /*= 1.0f*/)
{
	float Damage = GetAbilityStat(BaseDamageRow) * Multiplier;
	float Scaling = GetAbilityStat(CoefficientRow);

	return UAgoraAbilityLibrary::SetDamageSpecValues(SpecHandle, Damage, Scaling, DamageType);
}

FGameplayEffectSpecHandle UAgoraGameplayAbility::GetScaledDamageSpec(EDamageType DamageType, FName CoefficientRow /*= FName("DamageCoefficient")*/, FName BaseDamageRow /*= "damage"*/, float Multiplier)
{
	float Damage = GetAbilityStat(BaseDamageRow) * Multiplier;
	float Scaling = GetAbilityStat(CoefficientRow);

	return UAgoraAbilityLibrary::GetDamageSpec(MakeInstancedEffectContext(), Damage, Scaling, DamageType);
}

const TSet<AActor*> UAgoraGameplayAbility::ApplyScaledDamageInArea(UPrimitiveComponent* Area, FAgoraSweepQueryParams QuerySweepParams, EDamageType DamageType, FName CoefficientRow /*= FName("DamageCoefficient")*/, FName BaseDamageRow /*= "damage"*/, float Multiplier)
{
	FGameplayEffectSpecHandle DamageSpec = GetScaledDamageSpec(DamageType, CoefficientRow, BaseDamageRow, Multiplier);

	return UAgoraAbilityLibrary::ApplyEffectInArea(Area, DamageSpec, QuerySweepParams, GetActorInfo().OwnerActor.Get());
}

float UAgoraGameplayAbility::GetAbilityStat(FName StatRowName) const
{
	return GetAbilityStatCurve(StatRowName).GetValueAtLevel(GetAbilityLevel());
}

FName UAgoraGameplayAbility::GetCooldownRowName() const
{
	return CooldownRowName;
}

UGameplayEffect* UAgoraGameplayAbility::GetCooldownGameplayEffect() const
{
	if (CooldownGEClass)
	{
		return CooldownGEClass->GetDefaultObject<UCooldownGameplayEffect>();
	}

	return nullptr;
}

FScalableFloat UAgoraGameplayAbility::GetAbilityStatCurve(FName StatRowName) const
{
	FScalableFloat Scaled(1);
	FCurveTableRowHandle CurveHandle;
	CurveHandle.CurveTable = AbilityLevelTable;
	CurveHandle.RowName = StatRowName;
	Scaled.Curve = CurveHandle;

	return Scaled;
}

const FGameplayTagContainer* UAgoraGameplayAbility::GetCooldownTags() const
{
	if (!CooldownTags.IsValid())
	{
		return Super::GetCooldownTags();
	}

	return &CooldownTags;
}

FGameplayTagCountContainer& UAgoraGameplayAbility::GetTagCountContainer()
{
	return TagCountContainer;
}

void UAgoraGameplayAbility::LogAbility(const FString& InString /*= FString(TEXT("Hello"))*/, EAbilityLogType AbilityLogType /*= EAbilityLogType::AgoraAbility*/, bool bPrintToScreen /*= true*/, bool bPrintToLog /*= true*/, FLinearColor TextColor /*= FLinearColor(0.0, 0.66, 1.0)*/, float Duration /*= 2.f*/)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) // Do not Print in Shipping or Test

    FString AbilityLogString;
    AbilityLogString += GetNameSafe(GetActorInfo().OwnerActor.Get()) + "->" + GetNameSafe(this) + ": ";
    AbilityLogString += InString;

    TRACE(AgoraAbility, Display, "%s", *AbilityLogString);
     
    if (bPrintToScreen)
    {
        if (GAreScreenMessagesEnabled)
        {
            if (GConfig && Duration < 0)
            {
                GConfig->GetFloat(TEXT("Kismet"), TEXT("PrintStringDuration"), Duration, GEngineIni);
            }
            GEngine->AddOnScreenDebugMessage((uint64)-1, Duration, TextColor.ToFColor(true), AbilityLogString);
        }
        else
        {
            UE_LOG(AgoraAbility, Warning, TEXT("Screen messages disabled (!GAreScreenMessagesEnabled).  Cannot print to screen."));
        }
    }
   
#endif
}

void UAgoraGameplayAbility::LogAbility_Warning(const FString& InString /*= FString(TEXT("Hello"))*/, EAbilityLogType AbilityLogType /*= EAbilityLogType::AgoraAbility*/, bool bPrintToScreen /*= true*/, bool bPrintToLog /*= true*/, FLinearColor TextColor /*= FLinearColor(0.0, 0.66, 1.0)*/, float Duration /*= 2.f*/)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) // Do not Print in Shipping or Test

    FString AbilityLogString;
    AbilityLogString += GetNameSafe(GetActorInfo().OwnerActor.Get()) + "->" + GetNameSafe(this) + ": ";
    AbilityLogString += InString;

    TRACE(AgoraAbility, Warning, "%s", *AbilityLogString);
   
    if (bPrintToScreen)
    {
        if (GAreScreenMessagesEnabled)
        {
            if (GConfig && Duration < 0)
            {
                GConfig->GetFloat(TEXT("Kismet"), TEXT("PrintStringDuration"), Duration, GEngineIni);
            }
            GEngine->AddOnScreenDebugMessage((uint64)-1, Duration, TextColor.ToFColor(true), AbilityLogString);
        }
        else
        {
            UE_LOG(AgoraAbility, Warning, TEXT("Screen messages disabled (!GAreScreenMessagesEnabled).  Cannot print to screen."));
        }
    }

#endif
}

void UAgoraGameplayAbility::LogAbility_Error(const FString& InString /*= FString(TEXT("Hello"))*/, EAbilityLogType AbilityLogType /*= EAbilityLogType::AgoraAbility*/, bool bPrintToScreen /*= true*/, bool bPrintToLog /*= true*/, FLinearColor TextColor /*= FLinearColor(0.0, 0.66, 1.0)*/, float Duration /*= 2.f*/)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) // Do not Print in Shipping or Test

    FString AbilityLogString;
    AbilityLogString += GetNameSafe(GetActorInfo().OwnerActor.Get()) + "->" + GetNameSafe(this) + ": ";
    AbilityLogString += InString;

    TRACE(AgoraAbility, Error, "%s", *AbilityLogString);

    if (bPrintToScreen)
    {
        if (GAreScreenMessagesEnabled)
        {
            if (GConfig && Duration < 0)
            {
                GConfig->GetFloat(TEXT("Kismet"), TEXT("PrintStringDuration"), Duration, GEngineIni);
            }
            GEngine->AddOnScreenDebugMessage((uint64)-1, Duration, TextColor.ToFColor(true), AbilityLogString);
        }
        else
        {
            UE_LOG(AgoraAbility, Warning, TEXT("Screen messages disabled (!GAreScreenMessagesEnabled).  Cannot print to screen."));
        }
    }

#endif
}


void UAgoraGameplayAbility::LogAbility_Fatal(const FString& InString /*= FString(TEXT("Hello"))*/, EAbilityLogType AbilityLogType /*= EAbilityLogType::AgoraAbility*/)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) // Do not Print in Shipping or Test

    FString AbilityLogString;
    AbilityLogString += GetNameSafe(GetActorInfo().OwnerActor.Get()) + "->" + GetNameSafe(this) + ": ";
    AbilityLogString += InString;

    TRACE(AgoraAbility, Fatal, "%s", *AbilityLogString);

#endif
}