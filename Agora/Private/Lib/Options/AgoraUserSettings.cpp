// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraUserSettings.h"
#include "AgoraOptionTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Lib/AgoraBlueprintFunctionLibrary.h"
#include "Lib/AgoraDataSingleton.h"

void UAgoraUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	Super::ApplySettings(bCheckForCommandLineOverrides);
}

void UAgoraUserSettings::SetMouseSensitivity(float Sensitivity)
{
	MouseSensitivity = Sensitivity;
}

float UAgoraUserSettings::GetMouseSensitivity()
{
	return MouseSensitivity;
}

void UAgoraUserSettings::ApplyMouseSensitivity(APlayerController* PC)
{
	if (PC && PC->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!PC || !PC->PlayerInput)
	{
		TRACE(Agora, Warning, "Apply mouse sensitivity called without a valid Player Controller or Player Input");
		return;
	}

	PC->PlayerInput->SetMouseSensitivity(MouseSensitivity);
	SaveConfig(CPF_Config, *GGameUserSettingsIni);
}

void UAgoraUserSettings::ApplySoundSettings(UObject* WorldContextObject)
{
	UAgoraDataSingleton* Data = UAgoraBlueprintFunctionLibrary::GetGlobals();
	USoundMix* GlobalSoundMix = Data->GetGlobalSoundMix();

	UGameplayStatics::SetBaseSoundMix(WorldContextObject, GlobalSoundMix);
	UGameplayStatics::PushSoundMixModifier(WorldContextObject, GlobalSoundMix);

	// In BP I needed a delay here, otherwise it didn't set correctly (for PIE only?)

	InitSoundChannel(WorldContextObject, ESoundChannel::Master);
	InitSoundChannel(WorldContextObject, ESoundChannel::Effects);
	InitSoundChannel(WorldContextObject, ESoundChannel::Music);
}

void UAgoraUserSettings::SetChannelVolume(UObject* WorldContextObject, ESoundChannel Channel, float Volume, bool bSaveConfig)
{
	UAgoraDataSingleton* Data = UAgoraBlueprintFunctionLibrary::GetGlobals();
	USoundClass* SoundClass = nullptr;

	switch (Channel)
	{
	case ESoundChannel::Master:
		SoundClass = Data->MasterSoundClass;
		break;
	case ESoundChannel::Effects:
		SoundClass = Data->EffectsSoundClass;
		break;
	case ESoundChannel::Music:
		SoundClass = Data->MusicSoundClass;
		break;
	}

	SoundOptions.SetChannelVolume(Channel, Volume);

	USoundMix* GlobalSoundMix = Data->GetGlobalSoundMix();
	UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, GlobalSoundMix, SoundClass, Volume);

	if (bSaveConfig)
	{
		SaveConfig(CPF_Config, *GGameUserSettingsIni);
	}
}

FSoundOptions UAgoraUserSettings::GetSoundOptions()
{
	return SoundOptions;
}

UAgoraUserSettings* UAgoraUserSettings::Get()
{
	if (GEngine)
	{
		UAgoraUserSettings* UserSettings = Cast<UAgoraUserSettings>(GEngine->GetGameUserSettings());
		if (!UserSettings)
		{
			UE_LOG(LogTemp, Error, TEXT("Attempted to get user settings without the class set!"));
		}

		return UserSettings;
	}

	UE_LOG(LogTemp, Warning, TEXT("Attempted to get user settings before engine was valid"));

	return nullptr;
}

void UAgoraUserSettings::InitSoundChannel(UObject* WorldContextObject, ESoundChannel Channel)
{
	SetChannelVolume(WorldContextObject, Channel, SoundOptions.GetChannelVolume(Channel), false);
}
