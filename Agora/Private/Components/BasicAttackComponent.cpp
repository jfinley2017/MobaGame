// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "BasicAttackComponent.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"
#include "Agora.h"
#include "Engine/Engine.h"
#include "BasicAttacker.h"
#include "AgoraBlueprintFunctionLibrary.h"

UBasicAttackComponent::UBasicAttackComponent()
{
    SetIsReplicated(true);
}

void UBasicAttackComponent::ResetCombo()
{
    SetComboIndex(0);
}

uint8 UBasicAttackComponent::GetPreviousComboIndex()
{
	if (CurrentComboIndex == 0) return ComboSequence.Num() - 1;
	return CurrentComboIndex - 1;
}

FBasicAttackAnimationHandle UBasicAttackComponent::GetCurrentAttackMontage(bool bProgressCombo)
{
    FBasicAttackAnimationHandle OutHandle = ComboSequence[CurrentComboIndex];
    if (bProgressCombo) SetComboIndex(CurrentComboIndex + 1);
    return OutHandle;
}

float UBasicAttackComponent::GetCurrentBasicAnimPlayrate()
{
	//alpaca: The idea of slowing down a basic attack animation to match the basic attack cooldown shouldn't apply to ranged autos.
	//so we have now this flag. set it in the character's BP.
	if (bRanged)
	{
		return 1.0; //note that you can still control the montage's playrate within the montage BP itself, if desired. Doing this decouples the auto cooldown from the animation.
	}


    // note: there should be an interface/attribute base class that has the most typical stat getter/setters at some point
	float SuggestedAnimPlayRate = GetSuggestedMaxAnimPlayRate();
    // requiring that the animation always is longer than the actual cooldown
    // this logic could become a bit more complex potentially, eg if the cooldown is shorter than the time the animation takes at the MaxAnimPlayRate, we need not
    // apply this buffer
    return SuggestedAnimPlayRate - ATTACK_COMBO_BUFFER;
}

float UBasicAttackComponent::GetSuggestedMaxAnimPlayRate()
{
	return UAgoraBlueprintFunctionLibrary::CalculateMontagePlayRate(GetCurrentAttackMontage(false).AttackAnimation, IBasicAttacker::Execute_GetBasicAttackCooldown(GetOwner()), EClampType::ClampBoth, 0.0f, GetCurrentAttackMontage(false).MaxAnimPlayrate);
}

void UBasicAttackComponent::SetComboIndex(uint8 NewIndex)
{
	CurrentComboIndex = NewIndex % ComboSequence.Num();
    TRACE(AgoraCharacter, Verbose, "%s NewPrimaryFireIndex : %d", *GetName(), CurrentComboIndex);
}

void UBasicAttackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UBasicAttackComponent, CurrentComboIndex, COND_SkipOwner);
}