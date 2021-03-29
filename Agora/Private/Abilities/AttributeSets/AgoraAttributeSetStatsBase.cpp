// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.


#include "AgoraAttributeSetStatsBase.h"


#include "GameplayEffect.h" 
#include "GameplayEffectExtension.h" // DamageModCallbackData

#include "Net/UnrealNetwork.h" // Replication

UAgoraAttributeSetStatsBase::UAgoraAttributeSetStatsBase():
    Mana(1.0f),
    MaxMana(1.0f),
    ManaRegen(1.0f),
    PhysicalPower(0.0f),
    MagicalPower(0.0f),
    CleavePercentage(0.0f),
    AttackSpeedBase(0.0f),
    AttackSpeedModifier(0.0f),
	CooldownReduction(0.0f),
    CriticalHitChance(0.0f),
    CriticalHitDamageModifier(2.0f)
{
    
}

float UAgoraAttributeSetStatsBase::GetModifiedAttackSpeed()
{
    return (AttackSpeedBase.GetCurrentValue() * (1.0 + AttackSpeedModifier.GetCurrentValue()));
}

float UAgoraAttributeSetStatsBase::CalculateBasicAttackCooldown() const
{
    // .4 is min attack speed that i set atm, this will probbably have to change, or at least be moved to a move global place
    // where GD can more easily define the slowest that something is allowed to attack at
    return FMath::Max(1.0f / ((AttackSpeedBase.GetCurrentValue() * (1.0 + AttackSpeedModifier.GetCurrentValue()))), 0.4);
}

void UAgoraAttributeSetStatsBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, MaxMana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, ManaRegen, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, PhysicalPower, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, MagicalPower, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, AttackSpeedModifier, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, AttackSpeedBase, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, CooldownReduction, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, CleavePercentage, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, CriticalHitChance, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetStatsBase, CriticalHitDamageModifier, COND_None, REPNOTIFY_Always);


}


void UAgoraAttributeSetStatsBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetMaxManaAttribute())
    {
        AdjustAttributeForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
    }

}

void UAgoraAttributeSetStatsBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data)
{
    Super::PostGameplayEffectExecute(Data);

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
    FGameplayAttribute ModifiedDataAttribute = Data.EvaluatedData.Attribute;

    if (ModifiedDataAttribute == GetManaAttribute() || ModifiedDataAttribute == GetMaxManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
    }

	if (ModifiedDataAttribute == GetCooldownReductionAttribute())
	{
		SetCooldownReduction(FMath::Clamp(GetCooldownReduction(), -0.3f, 0.3f));
	}
    
}
void UAgoraAttributeSetStatsBase::OnRep_Mana()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, Mana);
}

void UAgoraAttributeSetStatsBase::OnRep_MaxMana()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, MaxMana);
}

void UAgoraAttributeSetStatsBase::OnRep_ManaRegen()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, ManaRegen);
}

void UAgoraAttributeSetStatsBase::OnRep_AttackSpeedBase()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, AttackSpeedBase);
}

void UAgoraAttributeSetStatsBase::OnRep_AttackSpeedModifier()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, AttackSpeedModifier);
}

void UAgoraAttributeSetStatsBase::OnRep_CooldownReduction()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, CooldownReduction);
}

void UAgoraAttributeSetStatsBase::OnRep_PhysicalPower()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, PhysicalPower);
}

void UAgoraAttributeSetStatsBase::OnRep_MagicalPower()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, MagicalPower);
}

void UAgoraAttributeSetStatsBase::OnRep_CleavePercentage()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, CleavePercentage);
}

void UAgoraAttributeSetStatsBase::OnRep_CriticalHitChance()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, CriticalHitChance);
}

void UAgoraAttributeSetStatsBase::OnRep_CriticalHitDamageModifier()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetStatsBase, CriticalHitDamageModifier);
}