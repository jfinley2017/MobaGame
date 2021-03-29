// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraTargetActor_Trace.h"
#include "AgoraTargetActor_GroundTrace.generated.h"

class AActor;
class AAIController;

/**
 * This is a hack/copy paste job, due for major refactoring when it comes time to care about AI
 * We need to consider some solution to not constantly spawning the target actors, due to the cost of actor spawning (should also validate this claim)
 *
 * This originally extended ground trace, but too many things had to change
 */
UCLASS()
class AGORA_API AAgoraTargetActor_GroundTrace : public AAgoraTargetActor_Trace
{
	GENERATED_BODY()
	
public:
	AAgoraTargetActor_GroundTrace(const FObjectInitializer& ObjectInitializer);

	/** Radius for a sphere or capsule. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Targeting)
	float EffectRadius;

	/**
	 * Traces normally like the crosshair, but then finds the ground and handles backing up in case this brings us out of range
	 */
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;
	
	virtual void InitializeReticle();
	virtual bool ShouldProduceTargetData() const override;
	virtual bool IsConfirmTargetingAllowed() override;

	/**
	 * Find the ground and return the a hit result at the closest ground target that is within range
	 */
	virtual void TraceDown(FHitResult& OutHitResult, FVector TraceFrom, FCollisionQueryParams Params);

protected:
	void SetIsValidTarget(bool bIsValid);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Targeting)
	bool bIsValidTarget = true;

	float TraceBackStepSize = 100.0f;
};
