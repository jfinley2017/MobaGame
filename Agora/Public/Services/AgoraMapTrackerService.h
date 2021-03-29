// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Components/AgoraMapIconComponent.h"
#include "AgoraMapTrackerService.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorRegisteredWithMapTrackerSignature, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorUnRegistedWithMapTrackerSignature, AActor*, Actor);

/**
 * 
 */
UCLASS()
class AGORA_API AAgoraMapTrackerService : public AInfo
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintAssignable, Category = "MapTracker")
    FOnActorRegisteredWithMapTrackerSignature OnActorRegistered;
    UPROPERTY(BlueprintAssignable, Category = "MapTracker")
    FOnActorUnRegistedWithMapTrackerSignature OnActorUnRegistered;

    UFUNCTION()
    void RegisterWithTracker(AActor* Actor);

    UFUNCTION()
    void UnRegisterWithTracker(AActor* Actor);
	
    UFUNCTION()
    void GetTrackedActors(TArray<AActor*>& Actors);

protected:

    TSet<AActor*> TrackedMapActors;

};
