// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityWorldReticle.h"
#include "AgoraTargetingReticle.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API AAgoraTargetingReticle : public AGameplayAbilityWorldReticle
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void InitializeRadius(float InRadius);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	float Radius = 100.0f;
};
