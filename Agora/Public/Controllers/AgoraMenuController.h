// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AgoraMenuController.generated.h"

class UAgoraGameLiftService;

/**
 * Controller for the main menu and such. Holds the gamelift service.
 */

UCLASS()
class AGORA_API AAgoraMenuController : public APlayerController
{
	GENERATED_BODY()
public:
	AAgoraMenuController();

	UPROPERTY(BlueprintReadOnly, Category = "GameLift")
	UAgoraGameLiftService* GameLiftService = nullptr;
};
