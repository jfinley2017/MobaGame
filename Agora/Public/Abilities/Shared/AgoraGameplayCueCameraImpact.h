// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "AgoraGameplayCueCameraImpact.generated.h"

class UParticleSystem;

/**
 * This mostly is a copy paste job from GameplayCueNotify_HitImpact, just without sound and to grab the camera comp
 */
UCLASS(Blueprintable)
class AGORA_API UAgoraGameplayCueCameraImpact : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:

	/** Does this GameplayCueNotify handle this type of GameplayCueEvent? */
	virtual bool HandlesEvent(EGameplayCueEvent::Type EventType) const override;

	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;

	/** Effects to play for weapon attacks against specific surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameplayCue)
		UParticleSystem* ParticleSystem;
};
