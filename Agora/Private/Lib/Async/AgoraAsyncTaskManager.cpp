// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraAsyncTaskManager.h"
#include "Agora.h"
#include "Engine/World.h"
#include "AgoraBlueprintAsyncActionBase.h"

DECLARE_CYCLE_STAT(TEXT("AsyncTaskGarbageCollection"), STAT_GCAsyncTasks, STATGROUP_AgoraAsyncActions);

int32 AgoraDumpAsyncTaskList = 0;

static FAutoConsoleVariableRef CVarAgoraDumpAsyncTaskList(
    TEXT("Agora.DumpAsyncTaskList"),
    AgoraDumpAsyncTaskList,
    TEXT("Dumps the current active async task list to the screen"));

int32 AgoraNumTasksToPrint = 15;

static FAutoConsoleVariableRef CVarAgoraDumpAsyncTaskListMaxCount(
    TEXT("Agora.DumpAsyncTaskListMaxCount"),
    AgoraNumTasksToPrint,
    TEXT("Maximum number of tasks to print"));

AAgoraAsyncTaskManager::AAgoraAsyncTaskManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f;
}

AAgoraAsyncTaskManager* AAgoraAsyncTaskManager::Get(UObject* WorldContextObject)
{
    static TWeakObjectPtr<AAgoraAsyncTaskManager> Manager = nullptr;

    if (!Manager.Get() && WorldContextObject && WorldContextObject->GetWorld())
    {
        if (WorldContextObject->GetWorld()->GetGameInstance())
        {
            Manager = WorldContextObject->GetWorld()->SpawnActor<AAgoraAsyncTaskManager>();
            WorldContextObject->GetWorld()->GetGameInstance()->RegisterReferencedObject(Manager.Get());
            TRACESTATIC(LogAgoraBlueprintAsyncActionBase, Log, "Async Task Manager spawned.");
        }
    }
    
    return Manager.Get();

}

void AAgoraAsyncTaskManager::AddTask(FLatentActionInfo LatentInfo, UAgoraBlueprintAsyncActionBase* Task)
{
    ActiveTasks.Add(FAgoraAsyncTaskListing(LatentInfo.UUID, LatentInfo.CallbackTarget, Task));
    TRACE(LogAgoraBlueprintAsyncActionBase, Verbose, "Registered %s with ID: %d CallbackTarget: %s with task manager.", *GetNameSafe(Task), LatentInfo.UUID, *GetNameSafe(LatentInfo.CallbackTarget));
}

void AAgoraAsyncTaskManager::RemoveTask(FLatentActionInfo LatentInfo, UAgoraBlueprintAsyncActionBase* Task)
{
    for (int32 i = 0; i < ActiveTasks.Num(); i++)
    {
        if (ActiveTasks[i].UUID == LatentInfo.UUID && ActiveTasks[i].CallbackTarget == LatentInfo.CallbackTarget)
        {
            ActiveTasks.RemoveAt(i);
        }
    }
}

UAgoraBlueprintAsyncActionBase*  AAgoraAsyncTaskManager::FindTask(FLatentActionInfo LatentInfo)
{
    for (FAgoraAsyncTaskListing& Listing : ActiveTasks)
    {
        if (Listing.UUID == LatentInfo.UUID && Listing.CallbackTarget == LatentInfo.CallbackTarget)
        {
            return Listing.Task;
        }
    }
    return nullptr;
}

void AAgoraAsyncTaskManager::Tick(float DeltaSeconds)
{
    
    SCOPE_CYCLE_COUNTER(STAT_GCAsyncTasks);

      if (AgoraDumpAsyncTaskList > 0)
      {
          FString TaskList = "";
          TaskList += FString::Printf(TEXT("Total Active Tasks: %d\n Displaying %d (Agora.DumpAsyncTaskListMaxCount)\n"), ActiveTasks.Num(), AgoraNumTasksToPrint);

          for (int32 i = 0; i < ActiveTasks.Num() && i < AgoraNumTasksToPrint; i++)
          {
              if (ActiveTasks[i].Task)
              {
                  TaskList += FString::Printf(TEXT("Name: %s -> %s \n"), *GetNameSafe(ActiveTasks[i].Task), *GetNameSafe(ActiveTasks[i].CallbackTarget));
              }
          }

          SCREENDEBUG_GREEN(5.0f, "%s", *TaskList);

      }


}

