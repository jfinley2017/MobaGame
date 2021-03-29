// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AgoraPlayerState.h"
#include "Lib/Options/AgoraOptionTypes.h"
#include "AgoraGameInstance.generated.h"

/**
 * Singleton with lifespan of the entire game process
 */
UCLASS()
class AGORA_API UAgoraGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UAgoraGameInstance(const FObjectInitializer &ObjectInitializer);

    // UGameInstance
	virtual void Init() override;
	virtual void OnStart() override;
    // ~UGameInstance   
protected:
};