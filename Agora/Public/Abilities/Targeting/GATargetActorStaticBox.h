// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "AgoraSweepLibrary.h"
#include "AgoraTypes.h"
#include "GATargetActorStaticBox.generated.h"



/**
 * A generic box-style targeting actor whose dimensions may represent a fixed hitbox for an ability.
 */
UCLASS()
class AGORA_API AGATargetActorStaticBox : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
public:
	AGATargetActorStaticBox();
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;
	virtual void Tick(float DeltaSeconds) override;

	//The half-dimensions of the hitbox.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "StaticBox")
	FVector HitBoxExtent = FVector(150, 75, 100);

	//The center of the hitbox relative to our hero.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "StaticBox")
	FVector RelativeHitBoxCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Cone")
	ETargetActorRotationType RotationType = ETargetActorRotationType::LookingRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Cone")
	ETargetActorLocationType LocationType = ETargetActorLocationType::LocationRelative;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Cone")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "StaticBox")
	FAgoraSweepQueryParams AgoraQueryParams;

	//Display the hitbox?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "StaticBox")
	bool DrawHitbox = false;

	FVector GetLocation();

	FRotator GetRotation();
};
