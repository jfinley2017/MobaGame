#include "AgoraLaneMinionAIController.h"
#include "Agora.h"

#include "Engine/Public/TimerManager.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BlackboardComponent.h"


AAgoraLaneMinionAIController::AAgoraLaneMinionAIController(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{

}

UObject* AAgoraLaneMinionAIController::GetTarget() const
{
    if (GetBlackboardComponent())
    {
        return GetBlackboardComponent()->GetValueAsObject(TargetBlackboardKeyName);
    }
    return nullptr;
}

void AAgoraLaneMinionAIController::SetTarget(UObject* NewTarget, float TargetTimeoutDuration /*= -1.0f*/, bool bLockTargetForDuration /*= false*/)
{
    if (TargetLockPolicy == ETargetLockPolicy::Locked)
    {
        TRACE(AgoraAIVerbose, Log, "%s tried to set target %s, but is currently locked on to %s", *GetNameSafe(GetPawn()), *GetNameSafe(GetPawn()), *GetNameSafe(GetTarget()));
        return;
    }
    else
    {
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsObject(TargetBlackboardKeyName, NewTarget);
            if (bLockTargetForDuration)
            {
                TRACE(AgoraAIVerbose, Log, "%s set target %s, locked for %f seconds", *GetNameSafe(GetPawn()), *GetNameSafe(NewTarget), TargetTimeoutDuration);
                TargetLockPolicy = ETargetLockPolicy::Locked;
            }
            GetWorld()->GetTimerManager().SetTimer(TargetDropTimer, this, &AAgoraLaneMinionAIController::ClearTarget, TargetTimeoutDuration, false, -1.0f);
        }
    }
}


void AAgoraLaneMinionAIController::ClearTarget()
{

    if (GetBlackboardComponent())
    {
        TargetLockPolicy = ETargetLockPolicy::NotLocked;
        GetBlackboardComponent()->SetValueAsObject(TargetBlackboardKeyName, nullptr);
    }

}
