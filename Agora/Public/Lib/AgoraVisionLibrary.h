// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AgoraVisionService.h"
#include "AgoraVisionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AgoraVision, Log, All);
DECLARE_STATS_GROUP(TEXT("AgoraVision"), STATGROUP_AgoraVision, STATCAT_Advanced);

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraVisionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static AAgoraVisionService* GetVisionService(UObject* WorldContext);
	
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraVisionLibrary")
	static bool HasVisionOf(UObject* WorldContextObject, const AActor* Viewer, const AActor* Subject);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraVisionLibrary")
    static bool LocalPlayerHasVisionOf(UObject* WorldContextObject, const AActor* Subject);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraVisionLibrary")
    static void GenerateVisionDebugString(UObject* WorldContextObject, FString& OutDebugString);
    
};
