#include "AgoraPlayerStart.h"


// Begin TeamInterface
ETeam AAgoraPlayerStart::GetTeam() const
{
	return Team;
}

void AAgoraPlayerStart::SetTeam(ETeam NewTeam)
{
	Team = NewTeam;
}

FOnTeamChangedSignature& AAgoraPlayerStart::GetTeamChangedDelegate()
{
    return TeamChanged;
}
// ~TeamInterface
