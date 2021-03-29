// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AgoraMapLibrary.generated.h"

class AAgoraMapTrackerService;
class UAgoraMapIconComponent;

DECLARE_STATS_GROUP(TEXT("AgoraMap"), STATGROUP_AgoraMap, STATCAT_Advanced);
DECLARE_LOG_CATEGORY_EXTERN(AgoraMap, Log, All);

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraMapLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraMapLibrary")
	static AAgoraMapTrackerService* GetMapTrackerService(UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraMapLibrary")
    static void GetTrackedActors(UObject* WorldContextObject, TArray<AActor*>& OutTrackedActors);

    UFUNCTION(BlueprintPure, Category = "AgoraMapLibrary")
    static UAgoraMapIconComponent* GetMapIconComponent(AActor* Actor);

};
