// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GATargetActorRadius.generated.h"


class UGameplayAbility;
/**
 * A carbon-copy of the built-in GameplayAbilityTargetActor_Radius, except this one actually draws a reticle around the avatar!
 */
UCLASS(Blueprintable, notplaceable)
class AGORA_API AGATargetActorRadius : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AGATargetActorRadius();
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ConfirmTargetingAndContinue() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Radius of target acquisition around the ability's start location. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Radius)
		float Radius;

protected:

	//mysteriously left out of the built-in radial GATargetActor....
	TWeakObjectPtr<AGameplayAbilityWorldReticle> ReticleActor;

	TArray<TWeakObjectPtr<AActor> >	PerformOverlap(const FVector& Origin);

	FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<TWeakObjectPtr<AActor>>& Actors, const FVector& Origin) const;
};
