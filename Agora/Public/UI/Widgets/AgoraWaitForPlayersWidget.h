// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AgoraWaitForPlayersWidget.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraWaitForPlayersWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Agora|WaitForPlayers")
	void ReceiveNumConnectedPlayersChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Agora|WaitForPlayers")
	void ReceiveMatchStartCountdown(float CountdownDuration);

	// Begin UUserWidget
	virtual void NativeConstruct();
	// End UUserWidget
};
