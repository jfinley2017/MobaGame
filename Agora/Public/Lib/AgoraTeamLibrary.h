// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AgoraAffiliatedWidget.h"
#include "AgoraTeamLibrary.generated.h"

class AAgoraTeamService;
class AAgoraTeam;
class AAgoraPlayerState;

DECLARE_LOG_CATEGORY_EXTERN(AgoraTeam, Log, All);
DECLARE_STATS_GROUP(TEXT("AgoraTeam"), STATGROUP_AgoraTeam, STATCAT_Advanced);

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraTeamLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    static AAgoraTeamService* GetAgoraTeamService(UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraTeamLibrary")
    static AAgoraTeam* GetTeamByID(UObject* WorldContextObject, ETeam TeamID);

    UFUNCTION(BlueprintPure, Category = "AgoraTeamLibrary")
    static FString GetTeamNameFromID(ETeam TeamID);

    UFUNCTION(BlueprintPure, Category = "AgoraTeamLibrary")
    static ETeam GetTeam(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "AgoraTeamLibrary")
    static bool IsFriendly(const AActor* ActorOne, const AActor* ActorTwo);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraTeamLibrary")
    static void GenerateTeamServiceDebugString(UObject* WorldContextObject, FString& OutDebugString);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraTeamLibrary")
    static void GetActorsOnTeam(UObject* WorldContextObject, ETeam TeamID, TArray<AActor*>& Actors);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraTeamLibrary")
    static void GetHeroesOnTeam(UObject* WorldContextObject, ETeam TeamID, TArray<AAgoraHeroBase*>& Heroes);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraTeamLibrary")
    static void GetPlayersOnTeam(UObject* WorldContextObject, ETeam TeamID, TArray<APlayerState*>& OutPlayersForTeam);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraTeamLibrary")
    static EOwnerRelationship DetermineRelationshipToLocalPlayer(UObject* WorldContextObject, AActor* Actor);

};
