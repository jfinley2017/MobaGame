// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraTeamActor.h"

ETeam AAgoraTeamActor::GetTeam() const
{
	return Team;
}

void AAgoraTeamActor::SetTeam(ETeam NewTeam)
{
	Team = NewTeam;
}

FOnTeamChangedSignature& AAgoraTeamActor::GetTeamChangedDelegate()
{
	return TeamChanged;
}
