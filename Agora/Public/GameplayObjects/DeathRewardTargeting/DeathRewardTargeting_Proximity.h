// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeathRewardTargetInterface.h"
#include "DeathRewardTargeting_Proximity.generated.h"

class AActor;

/**
 * Get instigator friendly targets within a certain radius
 * This can be subclassed multiple times to define particular radiuses
 * Ideally we could parameterize this class somehow, but it would require something like templates, or editor tricks
 */
UCLASS()
class AGORA_API UDeathRewardTargeting_Proximity : public UObject, public IDeathRewardTargetInterface
{
	GENERATED_BODY()
public:

	virtual void GetRewardTargets(TArray<AActor*>& OutTargets, FDamageContext Context) override;
};
