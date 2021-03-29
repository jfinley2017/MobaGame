// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_Footstep.generated.h"

class USoundCue;

/**
 * AnimNotify for footstep sounds.
 */
UCLASS()
class AGORA_API UAN_Footstep : public UAnimNotify
{
	GENERATED_BODY()

	UAN_Footstep();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	USoundCue* Sound;
};
