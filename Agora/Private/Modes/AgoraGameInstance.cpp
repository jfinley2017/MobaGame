// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "AgoraGameInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "GameplayAbilities/Public/AbilitySystemGlobals.h"
#include "Lib/Options/AgoraUserSettings.h"

UAgoraGameInstance::UAgoraGameInstance(const FObjectInitializer &ObjectInitializer)
{
	
}

void UAgoraGameInstance::Init()
{
    Super::Init();

    UAbilitySystemGlobals::Get().InitGlobalData();

}

void UAgoraGameInstance::OnStart()
{
	Super::OnStart();

	UAgoraUserSettings::Get()->ApplySoundSettings(this);
}

