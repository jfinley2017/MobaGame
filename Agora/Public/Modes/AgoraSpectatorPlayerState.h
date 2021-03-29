// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AgoraSpectatorPlayerState.generated.h"

/**
 * This is a player state ONLY for spectating
 * It is not used for the death kill cam spectating, rather for someone streaming a game or watching a replay
 */
UCLASS()
class AGORA_API AAgoraSpectatorPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	AAgoraSpectatorPlayerState();
protected:
	
};
