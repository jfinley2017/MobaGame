// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "GA_BasePrimary.h"
#include "Engine/Classes/Animation/AnimMontage.h"
#include "AgoraAttributeSetBase.h"
#include "Agora.h"

// @todo make this an interface maybe, so that basic attacks can be played on anything we want them to be played on
// EG. Minions/Orb/whatever
#include "AgoraCharacterBase.h"


void UGA_BasePrimary::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

}