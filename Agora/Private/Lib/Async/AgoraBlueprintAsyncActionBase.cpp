// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraBlueprintAsyncActionBase.h"
#include "Agora.h"

DEFINE_LOG_CATEGORY(LogAgoraBlueprintAsyncActionBase);
DEFINE_STAT(STAT_AgoraAsyncTaskCount);

UAgoraBlueprintAsyncActionBase::UAgoraBlueprintAsyncActionBase()
{
    SET_DWORD_STAT(STAT_AgoraAsyncTaskCount, ++GlobalAgoraAsyncTaskCount);
    if (GlobalAgoraAsyncTaskCount >= 750)
    {
        TRACE(LogAgoraBlueprintAsyncActionBase, Warning, "AgoraAsyncTask count is over 750. Current task count: %d", GlobalAgoraAsyncTaskCount);
    }
    
    TRACE(LogAgoraBlueprintAsyncActionBase, Verbose, "%s created. %d tasks currently active.", *GetName(), GlobalAgoraAsyncTaskCount);
}

UAgoraBlueprintAsyncActionBase::~UAgoraBlueprintAsyncActionBase()
{
}

void UAgoraBlueprintAsyncActionBase::InitTask(UObject* InWorldContextObject, FLatentActionInfo InLatentActionInfo)
{
    // We might be a reused task. 
    Cleanup();

    WorldContextObject = InWorldContextObject;
    LatentActionInfo = InLatentActionInfo;
}

void UAgoraBlueprintAsyncActionBase::Activate() 
{
    Super::Activate();

    TRACE(LogAgoraBlueprintAsyncActionBase, Verbose, "%s Activated.", *GetNameSafe(this));
    
    // Hack to cleanup tasks created during actor construction scripts.
    // In order for this to be properly removed, one must solve moving/placing/etc actors spawning a ton of tasks in editor which will not be deleted.
    if (!WorldContextObject.Get() || !WorldContextObject->GetWorld()->IsGameWorld())
    {
        MarkPendingKill();
        GEngine->ForceGarbageCollection(true);
    }
}

void UAgoraBlueprintAsyncActionBase::Cleanup()
{
    return;
}

void UAgoraBlueprintAsyncActionBase::SetReadyToDestroy()
{
    Cleanup();

    Super::SetReadyToDestroy();
}

void UAgoraBlueprintAsyncActionBase::BeginDestroy()
{
    Cleanup();

    SET_DWORD_STAT(STAT_AgoraAsyncTaskCount, --GlobalAgoraAsyncTaskCount);
    TRACE(LogAgoraBlueprintAsyncActionBase, Verbose, "%s Destroyed. %d tasks currently active.", *GetNameSafe(this), GlobalAgoraAsyncTaskCount);
    AAgoraAsyncTaskManager* TaskManager = AAgoraAsyncTaskManager::Get(WorldContextObject.Get());
    if (TaskManager)
    {
        TaskManager->RemoveTask(LatentActionInfo, this);
    }

    Super::BeginDestroy();
}

void UAgoraBlueprintAsyncActionBase::EndTask()
{
    Cleanup();
    MarkPendingKill();
}
