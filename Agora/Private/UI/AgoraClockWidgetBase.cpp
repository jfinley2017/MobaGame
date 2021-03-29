// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.


#include "AgoraClockWidgetBase.h"
#include "AgoraBlueprintFunctionLibrary.h"


void UAgoraClockWidgetBase::GetCurrentGameTimeAsString(FString& OutTimeAsString)
{
    // This value can be negative, but this is useful if we want to display some sort of countdown
    // for the game 'starting' (eg minions spawning)
    int32 NumberOfSecondsInGame = FMath::Abs(UAgoraBlueprintFunctionLibrary::GetTimeSinceStartOfGame(GetWorld()));

    // Actual calculation
    int32 Hour = FMath::FloorToFloat(NumberOfSecondsInGame / 3600);
    int32 Minute = FMath::FloorToFloat(NumberOfSecondsInGame % 3600 / 60);
    int32 Second = FMath::FloorToFloat(NumberOfSecondsInGame % 3600 % 60);

     // Ignore hour position if we're nowhere near an hour, format as H:MM:SS
     OutTimeAsString = Hour > 0 ?
        FString::Printf(TEXT("%d:%02d:%02d"), Hour, Minute, Second) : FString::Printf(TEXT("%02d:%02d"), Minute, Second);
    
}
