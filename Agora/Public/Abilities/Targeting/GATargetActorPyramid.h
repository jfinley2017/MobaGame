// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "AgoraSweepLibrary.h"
#include "AgoraTypes.h"
#include "GATargetActorPyramid.generated.h"

/**
* A flexible pyramidal target actor. Useful for shotgun-like abilities.
*/
UCLASS()
class AGORA_API AGATargetActorPyramid : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
public:
	AGATargetActorPyramid();
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;
	virtual void Tick(float DeltaSeconds) override;

	//The location of the tip of the pyramid relative to our hero.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	FVector RelativePyramidOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	float PyramidHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	ETargetActorRotationType RotationType = ETargetActorRotationType::LookingRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	ETargetActorLocationType LocationType = ETargetActorLocationType::SocketAttachment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	float BaseWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	float BaseHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	FVector2D Spread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	FAgoraSweepQueryParams AgoraQueryParams;

	//Display the pyramid when firing?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Pyramid")
	bool bDrawPyramid = false;

	FVector GetLocation();

	FRotator GetRotation();
};
