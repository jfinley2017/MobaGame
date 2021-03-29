// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraWaitForPlayersWidget.h"
#include "AgoraGameState.h"

void UAgoraWaitForPlayersWidget::NativeConstruct()
{
	Super::NativeConstruct();
	AAgoraGameState* AGS = GetWorld()->GetGameState<AAgoraGameState>();
	AGS->OnNumPlayersChanged.AddDynamic(this, &UAgoraWaitForPlayersWidget::ReceiveNumConnectedPlayersChanged);
}
