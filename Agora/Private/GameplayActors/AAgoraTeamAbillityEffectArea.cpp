// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AAgoraTeamAbillityEffectArea.h"
#include <Components/SphereComponent.h>
#include <GameplayEffect.h>

AAgoraTeamAbillityEffectArea::AAgoraTeamAbillityEffectArea(const FObjectInitializer& ObjectInitializer)
{
	bIsInfinite = true;
	Duration = -1.0f;
	EffectCauser = this;
}

ETeam AAgoraTeamAbillityEffectArea::GetTeam() const
{
	return Team;
}

void AAgoraTeamAbillityEffectArea::SetTeam(ETeam NewTeam)
{
	Team = NewTeam;
}

FOnTeamChangedSignature& AAgoraTeamAbillityEffectArea::GetTeamChangedDelegate()
{
	return OnTeamChanged;
}