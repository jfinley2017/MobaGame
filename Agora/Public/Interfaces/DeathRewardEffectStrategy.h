// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AgoraDeathRewardComponent.h"
#include <Interface.h>
#include "DeathRewardEffectStrategy.generated.h"

/**
 * 
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDeathRewardEffectStrategy : public UInterface
{
	GENERATED_BODY()
	
};

/**
 *
 */
class AGORA_API IDeathRewardEffectStrategy
{
	GENERATED_BODY()
public:
	UFUNCTION()
	virtual UGameplayEffect* GetRewardEffect(const TArray<AActor*>& InActors, FDamageContext Context) = 0;
};

