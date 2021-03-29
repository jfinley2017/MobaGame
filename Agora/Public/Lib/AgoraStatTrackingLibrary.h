// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/AgoraStatTracker.h"
#include "AgoraStatTrackingLibrary.generated.h"

class APlayerState;

DECLARE_STATS_GROUP(TEXT("AgoraStatTracker"), STATGROUP_AgoraStatTracker, STATCAT_Advanced);
DECLARE_LOG_CATEGORY_EXTERN(AgoraStatTracker, Log, All);

/**
 * Static interfaces for interacting with StatTrackingComponents. 
 */
UCLASS()
class AGORA_API UAgoraStatTrackingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    /**
     * Modifies the stat @StatName on the player @Player. Can create the stat if it doesn't already exist for this player.
     */
    UFUNCTION(BlueprintCallable, Category = "AgoraStatTracking")
    static void ModifyPlayerStat(APlayerState* Player, const FString& StatName, EStatModType ModifierType, float Value, bool CreateIfNoExist);
	
    /**
     * Retrieves the stat @StatName on the player @Player. Can create the stat if it doesn't already exist for this player.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraStatTracking")
    static float GetPlayerStat(APlayerState* Player, const FString& StatName, bool CreateIfNoExist);
	
    /**
     * Generates a string containing all stats owned by @Player.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraStatTracking")
    static void GetStatsForPlayer(APlayerState* Player, FString& OutString);

};
