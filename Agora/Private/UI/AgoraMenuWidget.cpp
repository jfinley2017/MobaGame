// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "AgoraMenuWidget.h"


void UAgoraMenuWidget::Setup()
{
	this->AddToViewport();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (PlayerController)
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(this->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PlayerController->SetInputMode(InputModeData);

		PlayerController->bShowMouseCursor = true;
	}
}

void UAgoraMenuWidget::Teardown()
{
	this->RemoveFromViewport();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (PlayerController)
	{
		FInputModeGameOnly InputModeData;

		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false;
	}
}

void UAgoraMenuWidget::JoinMatch()
{

}

