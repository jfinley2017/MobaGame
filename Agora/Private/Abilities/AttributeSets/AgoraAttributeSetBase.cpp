// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "AgoraAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h" 
#include "GameplayEffectExtension.h" // DamageModCallbackData

#include "AgoraBlueprintFunctionLibrary.h" // GetGameplayTag
#include "Agora.h" // Logging


UAgoraAttributeSetBase::UAgoraAttributeSetBase() :
    Super()
{

}

void UAgoraAttributeSetBase::PreAttributeChange(const FGameplayAttribute &Attribute, float& NewValue)
{
	// This is called whenever attributes change, so for max health we want to scale the current totals to match
	Super::PreAttributeChange(Attribute, NewValue);

	
}

void UAgoraAttributeSetBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData & Data)
{
    Super::PostGameplayEffectExecute(Data);

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
    FGameplayAttribute ModifiedDataAttribute = Data.EvaluatedData.Attribute;
   
  
}

void UAgoraAttributeSetBase::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
    UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
    const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
    if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
    {
        // Change current value to maintain the current Val / Max percent
        const float CurrentValue = AffectedAttribute.GetCurrentValue();
        float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

        AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
    }
}
