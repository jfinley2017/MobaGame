// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeathRewardEffectStrategy.h"
#include "DeathEffectStrategy_SplitExperience.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UDeathEffectStrategy_SplitExperience : public UObject, public IDeathRewardEffectStrategy
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	virtual UGameplayEffect* GetRewardEffect(const TArray<AActor*>& InActors, FDamageContext Context);
};
