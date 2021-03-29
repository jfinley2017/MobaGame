// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraOptionTypes.generated.h"

UENUM(BlueprintType)
enum class ESoundChannel : uint8
{
	Master,
	Effects,
	Music
};

USTRUCT(BlueprintType)
struct FInputOptions
{
	GENERATED_BODY();

public:
	FInputOptions() {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AgoraInputOptions")
	float MouseSensitivity = 0.5f;
};

USTRUCT(BlueprintType)
struct FSoundOptions
{
	GENERATED_BODY();

public:
	float GetChannelVolume(ESoundChannel Channel);
	void SetChannelVolume(ESoundChannel Channel, float Volume);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AgoraSoundOptions")
	float MasterVolume = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AgoraSoundOptions")
	float EffectsVolume = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AgoraSoundOptions")
	float MusicVolume = 0.5f;
};