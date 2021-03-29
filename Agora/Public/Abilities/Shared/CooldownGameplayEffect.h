// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CooldownGameplayEffect.generated.h"

/**
 * This class is effectively just a tag to indicate that it should be used for cooldowns. You must subclass this in order
 * for your GE to show up in AgoraAbility as a CD option
 */
UCLASS()
class AGORA_API UCooldownGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()
	
};
