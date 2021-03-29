// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor_Trace.h"
#include "GATargetActorLine.generated.h"

/**
 * A better version of the built-in SingleLineTrace.
 */
UCLASS(Blueprintable)
class AGORA_API AGATargetActorLine : public AGameplayAbilityTargetActor_Trace
{
	GENERATED_UCLASS_BODY()
protected:
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;

	UPROPERTY()
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
};
