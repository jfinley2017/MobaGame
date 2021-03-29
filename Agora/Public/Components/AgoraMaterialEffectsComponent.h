// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AgoraMaterialEffectsComponent.generated.h"

class UMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGORA_API UAgoraMaterialEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAgoraMaterialEffectsComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Set this float value for all materials, such as for hit flash
	void SetAllMaterialValue(FName FnName, float Val);

	UMeshComponent* OuterMesh;

public:	
	virtual void TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SetMeshMaterialsDynamic();

	float DisintegratePercent = 0.0f;
	float DamageFlashPercent = 0.0f;

	// Multiplied by DeltaSeconds
	UPROPERTY(EditDefaultsOnly, Category = "MaterialEffects")
	float HitFlashDecreaseStepSize = 3.0f;

	// Plays a red hit flash, max intensity = 1
	virtual void PlayHitFlash(float Intensity = 1.0f);

	virtual void PlayDeathDisintegrate();
		
};
