// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include <Engine/CollisionProfile.h>
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraTargetActor_Trace.generated.h"

class AAIController;
class AController;

static TAutoConsoleVariable<int32> CVarDebugTargetingTraces(
	TEXT("AG.DebugTargetingTraces"),
	0,
	TEXT("Non zero enables debug drawing shapes"),
	ECVF_Cheat
);

/**
 * Handles the complexities involved in third person shooter targeting traces
 * Built on the premise that the player's crosshair should be the source of truth 
 * in targeting. Works best for hitscan or fast projectiles. For slower projectiles
 * the results will be more confusing, as this will adjust the trajectory of the projectile
 * depending on what we are aiming at. 
 */
USTRUCT(BlueprintType)
struct FThirdPersonTrace
{
	GENERATED_BODY()
public:
	FThirdPersonTrace()
		: SourceActor(nullptr), CameraLocation(FVector::ZeroVector), CameraDirection(FVector::ZeroVector), TraceCenter(FVector::ZeroVector), MaxRange(-1.0f) {};

	FThirdPersonTrace(const AActor* SourceActor, const FVector& CameraLocation, const FVector& CameraDirection, const FVector& TraceCenter, float MaxRange);

	// Init the trace data using a player controller's view point
	FThirdPersonTrace(const AController* PC, const AActor* SourceActor, const FVector& TraceCenter, float MaxRange);

	virtual ~FThirdPersonTrace() {}

	UPROPERTY(BlueprintReadOnly)
	const AActor* SourceActor;

	UPROPERTY(BlueprintReadOnly)
	FVector CameraLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector CameraDirection;

	// This is the location that should be considered when calculating whether or not something is in range, and where the projectile will actually be spawned from
	UPROPERTY(BlueprintReadOnly)
	FVector TraceCenter;

	UPROPERTY(BlueprintReadOnly)
	float MaxRange;

	FVector GetTraceCenter() const;

	/**
	 * Default overload for convenience, trace against enemy actors, ignoring self
	 */
	bool Trace(FHitResult& OutHitLocation);

	/**
	 * @param OutHitLocation Output param representing the result of this trace
	 * @return True if we hit something blocking
	 */
	virtual bool Trace(FHitResult& OutHitLocation, const FAgoraSweepQueryParams& SweepParams, const FCollisionQueryParams& CollisionParams, ECollisionChannel CollisionChannel = ECC_Damage);

	/**
	 * @param OutMaxTrace Output param representing where we would trace to if nothing is hit
	 * @return False if the trace was not successful, this should normally not happen
	 */
	bool TraceMaxEnd(FVector& OutMaxTrace);

protected:
	UWorld* WorldContext;

	// This is the distance of the camera from the trace center projected along the camera direction axis
	// Aka the dot product of camera direction and distance from camera to trace center
	float CameraDistanceToCenterProjected();
};

/**
 * Base class for shared target actor functionality, such as handling tracing
 */
UCLASS(Blueprintable, notplaceable)
class AGORA_API AAgoraTargetActor_Trace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:
	AAgoraTargetActor_Trace(const FObjectInitializer& ObjectInitializer);

	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void InitializeReticle();
	virtual void Tick(float DeltaSeconds) override;

	virtual bool ShouldProduceTargetData() const override;

	/**
	 * @param Location Test location
	 * @return True if location is in range
	 */
	bool IsLocationInRange(const FVector& Location) const;

	// This defaults to only affecting enemies
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	FAgoraSweepQueryParams SweepParams;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	bool bIgnoreBlocks;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ConfirmTargetingAndContinue() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	float MaxRange;

	// Does the trace affect the aiming pitch
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	bool bTraceAffectsAimPitch;

protected:
	AAIController* GetAIController(AActor* InSourceActor) const;
	bool IsAI() const;
	FVector GetTraceCenter() const;

	virtual FHitResult PerformTrace(AActor* InSourceActor);

	FGameplayAbilityTargetDataHandle MakeTargetData(const FHitResult& HitResult) const;

	TWeakObjectPtr<AGameplayAbilityWorldReticle> ReticleActor;
};
