// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "Lib/Options/AgoraOptionTypes.h"
#include "AgoraUserSettings.generated.h"

/**
 * Handles user settings, saving them and applying them to files
 */
UCLASS(BlueprintType, config = GameUserSettings, configdonotcheckdefaults)
class AGORA_API UAgoraUserSettings : public UGameUserSettings
{
	GENERATED_BODY()
	
public:
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;


	//////////// INPUT
	UFUNCTION(BlueprintCallable, Category = "CustomUserSettings")
	void SetMouseSensitivity(float Sensitivity);

	UFUNCTION(BlueprintPure, Category = "CustomUserSettings")
	float GetMouseSensitivity();


	UFUNCTION(BlueprintCallable, Category = "CustomUserSettings")
	void ApplyMouseSensitivity(APlayerController* PC);


	/////////// SOUND

	UFUNCTION(BlueprintCallable, Category = "CustomUserSettings", meta = (WorldContext = WorldContextObject))
	void SetChannelVolume(UObject* WorldContextObject, ESoundChannel Channel, float Volume, bool bSaveConfig = true);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = WorldContextObject))
	void ApplySoundSettings(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "CustomUserSettings")
	FSoundOptions GetSoundOptions();

	/////////// UTILITY

	static UAgoraUserSettings* Get();
protected:

	UPROPERTY(Config)
	float MouseSensitivity = 0.1;

	UPROPERTY(Config)
	FSoundOptions SoundOptions;

	void InitSoundChannel(UObject* WorldContextObject, ESoundChannel Channel);
};
