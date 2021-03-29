// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/DeathRewardTargetInterface.h"
#include "DeathRewardTargeting_Team.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UDeathRewardTargeting_Team : public UObject, public IDeathRewardTargetInterface
{
	GENERATED_BODY()
public:

	virtual void GetRewardTargets(TArray<AActor*>& OutTargets, FDamageContext Context) override;
};
