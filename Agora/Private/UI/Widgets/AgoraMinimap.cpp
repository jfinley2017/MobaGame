// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraMinimap.h"
#include "AgoraMapLibrary.h"

DECLARE_CYCLE_STAT(TEXT("UpdateMinimapIcons"), STAT_UpdateMinimapIcons, STATGROUP_AgoraMap);

void UAgoraMinimap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    SCOPE_CYCLE_COUNTER(STAT_UpdateMinimapIcons);

    UpdateIcons(MyGeometry, InDeltaTime);

}
