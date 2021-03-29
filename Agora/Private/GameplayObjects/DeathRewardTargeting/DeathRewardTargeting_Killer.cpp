// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "DeathRewardTargeting_Killer.h"

void UDeathRewardTargeting_Killer::GetRewardTargets(TArray<AActor*>& OutTargets, FDamageContext Context)
{
	OutTargets.Add(Context.DamagingEffectContext.GetOriginalInstigator());
}
