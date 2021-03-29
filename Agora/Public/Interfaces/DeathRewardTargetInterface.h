// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AgoraDeathRewardComponent.h"
#include "DeathRewardTargetInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDeathRewardTargetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * These classes are meant to be used like static functions, ie all functions will be called on the CDO
 * The point is just to be able to select a different target strategy from the dropdown menu in details editor
 * This is an experiment to see if this is less annoying than maintaining an enum + function map, or if the extra
 * abstraction is actually just more confusing
 */
class AGORA_API IDeathRewardTargetInterface
{
	GENERATED_BODY()
public:
	UFUNCTION()
	virtual void GetRewardTargets(TArray<AActor*>& OutTargets, FDamageContext Context) = 0;
};
