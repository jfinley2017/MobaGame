// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "DeathRewardTargeting_Team.h"
#include "Interfaces/TeamInterface.h"
#include "Modes/AgoraGameState.h"
#include "Services/AgoraTeamService.h"
#include "AgoraHeroBase.h"

void UDeathRewardTargeting_Team::GetRewardTargets(TArray<AActor*>& OutTargets, FDamageContext Context)
{
	AActor* Inst = Context.DamageInstigator;

	ITeamInterface* InstigatorAsTeam = Cast<ITeamInterface>(Inst);
	ETeam InstigatorTeam = InstigatorAsTeam->GetTeam();

	AAgoraGameState* AGS = Cast<AAgoraGameState>(Inst->GetWorld()->GetGameState());
	AAgoraTeamService* TeamService = AGS->GetTeamService();

	if (TeamService)
	{
		if (InstigatorTeam != ETeam::Invalid && InstigatorTeam != ETeam::Neutral)
		{
			TArray<AAgoraHeroBase*> TeamHeroes;
			TeamService->GetHeroesOnTeam(InstigatorTeam, TeamHeroes);

			OutTargets.Append(TeamHeroes);
		}
		else
		{
			TRACE(Agora, Warning, "Invalid team used in get team reward targets");
		}
	}
	else
	{
		TRACE(Agora, Warning, "Team service not found");
	}
}
