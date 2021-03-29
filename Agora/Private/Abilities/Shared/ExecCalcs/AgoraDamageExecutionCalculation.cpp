// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "AgoraDamageExecutionCalculation.h"

#include "AttributeSets/AgoraAttributeSetStatsBase.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraTypes.h"
#include "AgoraGameplayTags.h"

struct AgoraDamageStatics
{
    // Source Attributes
    DECLARE_ATTRIBUTE_CAPTUREDEF(MagicalPower);
    DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalPower);
    DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
    DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamageModifier);

    // Target Attributes
    DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDefense);
    DECLARE_ATTRIBUTE_CAPTUREDEF(MagicalDefense);
    DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

    AgoraDamageStatics()
    {
        // Source Attributes
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAgoraAttributeSetStatsBase, MagicalPower, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAgoraAttributeSetStatsBase, PhysicalPower, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAgoraAttributeSetStatsBase, CriticalHitChance, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAgoraAttributeSetStatsBase, CriticalHitDamageModifier, Source, true);

        // Target Attributes
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAgoraAttributeSetHealth, PhysicalDefense, Target, false);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAgoraAttributeSetHealth, MagicalDefense, Target, false);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAgoraAttributeSetHealth, Damage, Target, false);
    }
};

static const AgoraDamageStatics& DamageStatics()
{
    // Return a new AgoraDamageStatics
    static AgoraDamageStatics OutDamageStatics;
    return OutDamageStatics;
}

UAgoraDamageExecutionCalculation::UAgoraDamageExecutionCalculation()
{

    // Calculation Attributes
    RelevantAttributesToCapture.Add(DamageStatics().MagicalDefenseDef);
    RelevantAttributesToCapture.Add(DamageStatics().PhysicalDefenseDef);
	RelevantAttributesToCapture.Add(DamageStatics().MagicalPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalPowerDef);
    RelevantAttributesToCapture.Add(DamageStatics().PhysicalDefenseDef);
    RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
    RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageModifierDef);

    // MetaAttributes
    RelevantAttributesToCapture.Add(DamageStatics().DamageDef);

}

void UAgoraDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{

    // Where we're calculating damage.
    // Currently is 'simple' and accounts for Magical / Physical damage.
    // Looking forward:
    // True damage
    // Penetration
    // Lifesteal? Is that a concern for this exec? Can we get away with plugging behavior like that in as a separate ability that responds
    //  to a gameplay event? Should abilities be concerned with collecting 
    
    // GameplayEffectSpec Data
    const FGameplayEffectSpec& OwningSpec = ExecutionParams.GetOwningSpec();

    FAgoraGameplayEffectContext* AgoraGameplayEffectContext = nullptr;
    FGameplayTagContainer* OutgoingContextTags = nullptr;

    if (OwningSpec.GetContext().IsValid())
    {
        AgoraGameplayEffectContext = static_cast<FAgoraGameplayEffectContext*>(OwningSpec.GetContext().Get());
        if (AgoraGameplayEffectContext)
        {
            OutgoingContextTags = AgoraGameplayEffectContext->GetContextTagsMutable();
        }

    }

    // Relevant Tags
    const FGameplayTagContainer* SourceTags = OwningSpec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = OwningSpec.CapturedTargetTags.GetAggregatedTags();

    // Ability Tags
    const UGameplayAbility* SourceGameplayAbility = OwningSpec.GetEffectContext().GetAbility();
    const FGameplayTagContainer* SourceAbilityTags = SourceGameplayAbility ? &SourceGameplayAbility->AbilityTags : nullptr;

    // Evaluation Parameters
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;
    
    // Target Attributes
    float TargetDefense = 0.0f;

    // Source Attributes
    float SourcePower = 0.0f;
    float SourceCritChance = 0.0f;
    float SourceCritModifier = 0.0f;

    // SetByCaller Data from Spec
    float BaseDamage = OwningSpec.GetSetByCallerMagnitude(UAgoraGameplayTags::SetByCallerBaseDamage(), true, 0.f);
    float ScalingModifier = OwningSpec.GetSetByCallerMagnitude(UAgoraGameplayTags::SetByCallerScalingValue(), true, 0.f);

    EDamageType AbilityDamageType = DetermineDamageType(OwningSpec);

    if (AbilityDamageType == EDamageType::DMGMagical)
    {
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().MagicalDefenseDef, EvaluationParameters, TargetDefense);
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().MagicalPowerDef, EvaluationParameters, SourcePower);
        if(OutgoingContextTags) OutgoingContextTags->AddTag(UAgoraGameplayTags::DamageTypeMagical());
    }
    else if (AbilityDamageType == EDamageType::DMGPhysical)
    {
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PhysicalDefenseDef, EvaluationParameters, TargetDefense);
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PhysicalPowerDef, EvaluationParameters, SourcePower);
        if(OutgoingContextTags) OutgoingContextTags->AddTag(UAgoraGameplayTags::DamageTypePhysical());
    }

    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, SourceCritChance);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageModifierDef, EvaluationParameters, SourceCritModifier);

    // Offense calculation

    float AggregateDamageDone = 0.0f;
    AggregateDamageDone += BaseDamage + (ScalingModifier * SourcePower);

    if (SourceAbilityTags && SourceAbilityTags->HasTag(UAgoraGameplayTags::CanCrit()))
    {
        // Here we crit
        if (SourceCritChance > 0.0f && UAgoraBlueprintFunctionLibrary::GenerateRandomFloat(0.0f,1.0f) <= SourceCritChance)
        {
            AggregateDamageDone *= SourceCritModifier;
            // TODO apply tag to source GE saying that this is a crit
            if(OutgoingContextTags)  OutgoingContextTags->AddTag(UAgoraGameplayTags::CriticalHit());
        }
    }

    // Defense Calculations
    // TODO: True damage (another tag on the ability?), penetration in general
    AggregateDamageDone = AggregateDamageDone / ((TargetDefense + 100) / 100);

    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AgoraDamageStatics().DamageProperty, EGameplayModOp::Override, AggregateDamageDone));
}

EDamageType UAgoraDamageExecutionCalculation::DetermineDamageType(const FGameplayEffectSpec& Spec)
{

    EDamageType ReturnedDamageType = EDamageType::DMGInvalid;

    //if (!Spec) { return ReturnedDamageType; }

	FGameplayTagContainer SpecTags;
	Spec.GetAllAssetTags(SpecTags);
    if (SpecTags.HasTag(UAgoraGameplayTags::DamageTypeMagical()))
    {
        ReturnedDamageType = EDamageType::DMGMagical;
    }
    else if (SpecTags.HasTag(UAgoraGameplayTags::DamageTypePhysical()))
    {
        ReturnedDamageType = EDamageType::DMGPhysical;
    }

    return ReturnedDamageType;
}
