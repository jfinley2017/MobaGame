// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Engine/LatentActionManager.h"
#include "AgoraAsyncTaskManager.generated.h"

class UAgoraBlueprintAsyncActionBase;

USTRUCT()
struct FAgoraAsyncTaskListing
{
    GENERATED_BODY()
    
    FAgoraAsyncTaskListing() {}
    
    FAgoraAsyncTaskListing(int32 InUUID, UObject* InCallbackTarget, UAgoraBlueprintAsyncActionBase* InTask) 
        : UUID(InUUID), 
          Task(InTask),
          CallbackTarget(InCallbackTarget)
    {}
    
    UPROPERTY()
    int32 UUID;

    UPROPERTY()
    UAgoraBlueprintAsyncActionBase* Task;

    UPROPERTY()
    UObject* CallbackTarget;

};


/**
 * 
 */
UCLASS()
class AGORA_API AAgoraAsyncTaskManager : public AInfo
{
	GENERATED_BODY()
	

public:

    AAgoraAsyncTaskManager();

    // AActor
    virtual void Tick(float DeltaSeconds) override;
    // ~AActor
    
    /**
     * Retrieves or creates an async task manager. Registered with game instance.
     */
    static AAgoraAsyncTaskManager* Get(UObject* WorldContextObject);

    /**
     * Adds a new task wit the specified unique ID
     */
    UFUNCTION()
    void AddTask(FLatentActionInfo LatentInfo, UAgoraBlueprintAsyncActionBase* Task);

    UFUNCTION()
    void RemoveTask(FLatentActionInfo LatentInfo, UAgoraBlueprintAsyncActionBase* Task);

    /**
     * Finds a task with the specified unique ID
     */
    UFUNCTION()
    UAgoraBlueprintAsyncActionBase* FindTask(FLatentActionInfo LatentInfo);

protected:
	
    
    TArray<FAgoraAsyncTaskListing> ActiveTasks;

};
