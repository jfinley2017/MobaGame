// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AgoraClockWidgetBase.generated.h"

/**
 * Visual representation of the amount of time passed in a game.
 */
UCLASS()
class AGORA_API UAgoraClockWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:

    /**
     * Returns the time passed in the game, converted to a string
     */
    UFUNCTION(BlueprintPure, Category = "AgoraClock")
    virtual void GetCurrentGameTimeAsString(FString& OutTimeAsString);

protected:


};
