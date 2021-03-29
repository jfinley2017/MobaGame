// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "AgoraTypes.h"
#include "AgoraWorldSettings.generated.h"

class USoundCue;

/**
 * Settings specific to the entire game world
 */
UCLASS()
class AGORA_API AAgoraWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = GameMode)
	float GetGameBeginCountdownDuration();

	// For AgoraGameMode - Specifies the last tower, ie if you want to have a map with only core + inhibitor. Towers behind the last tower will be set invulnerable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameMode)
	ETowerType LastTower = ETowerType::T1Tower;

	//Sound stuff

	//Right now we just have this play for everybody whenever a player dies.
	//Soon we're probably going to want separate sounds for if an ally dies vs an enemy dies vs if YOU die
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameMode)
	USoundCue* PlayerDeathSound;

	//More sounds

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameMode)
	USoundCue* TowerDeathSound;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameMode)
	float GameBeginCountdownDuration = 10.f;

};
