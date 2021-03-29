// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/LatentActionManager.h"
#include "Engine/World.h"
#include "Agora.h"
#include "AgoraAsyncTaskManager.h"

#include "AgoraBlueprintAsyncActionBase.generated.h"


DECLARE_STATS_GROUP(TEXT("AsyncTasks"), STATGROUP_AgoraAsyncActions, STATCAT_Advanced);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("AsyncTask Count"), STAT_AgoraAsyncTaskCount, STATGROUP_AgoraAsyncActions, );

DECLARE_LOG_CATEGORY_EXTERN(LogAgoraBlueprintAsyncActionBase, Log, All);

static const int32 WITH_ASYNC_MANAGER = 0;
static int32 GlobalAgoraAsyncTaskCount = 0;

/**
 * Provides a clean interface for exposing delegates to Blueprint whilst tracking usage.
 * Should not currently be used in source.
 */
UCLASS(Abstract, BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class AGORA_API UAgoraBlueprintAsyncActionBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

    template <class T>
    static T* NewAgoraAsyncTask(UObject* InWorldContextObject, FLatentActionInfo InLatentActionInfo)
    {
        check(InWorldContextObject);

        AAgoraAsyncTaskManager* TaskManager = AAgoraAsyncTaskManager::Get(InWorldContextObject);
        if (TaskManager)
        {
            UAgoraBlueprintAsyncActionBase* Task = TaskManager->FindTask(InLatentActionInfo);
            if (Task)
            {
                Task->InitTask(InWorldContextObject, InLatentActionInfo);
                return Cast<T>(Task);
            }

            T* MyObj = NewObject<T>();
            MyObj->InitTask(InWorldContextObject, InLatentActionInfo);
            TaskManager->AddTask(InLatentActionInfo, Cast<UAgoraBlueprintAsyncActionBase>(MyObj));
            return MyObj;
        }
        
        // Should only get here in editor, as we can't spawn a task manager in editor.
        T* MyObj = NewObject<T>();
        MyObj->InitTask(InWorldContextObject, InLatentActionInfo);
        return MyObj;

    }

    UAgoraBlueprintAsyncActionBase();
    ~UAgoraBlueprintAsyncActionBase();

    //UObject
    virtual void BeginDestroy() override;
    //~UObject

    UFUNCTION(BlueprintCallable, Category = "AgoraAsyncTask")
    void EndTask();

    /** Called when execution reaches this node. Good places to setup delegates/etc. */
    virtual void Activate() override;

    /** Called on Init (re-initializing a task) and prior to death. Remove any references that are used by this task here. */
    virtual void Cleanup();

    /** Called before destroying this node. Good place to cleanup. Calls Cleanup(). */
    virtual void SetReadyToDestroy() override final;

protected:

    FLatentActionInfo LatentActionInfo;
    TWeakObjectPtr<UObject> WorldContextObject;

private:

    /** Called every time execution reaches this object. */
    void InitTask(UObject* WorldContextObject, FLatentActionInfo InLatentActionInfo);

};
