// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AgoraMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	// Setting the InputMode to UIOnly
	UFUNCTION(BlueprintCallable)
	void Setup();

	// Manual function for doing the same
	UFUNCTION(BlueprintCallable)
	void Teardown();

	UFUNCTION(BlueprintCallable)
	void JoinMatch();

protected:
};