// For internal use only

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AgoraLaneMinionAIController.generated.h"

UENUM(BlueprintType)
enum class ETargetLockPolicy : uint8
{
    NotLocked,
    Locked
};

UCLASS()
class AGORA_API AAgoraLaneMinionAIController : public AAIController
{
	GENERATED_BODY()

public:

    AAgoraLaneMinionAIController(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "AgoraMinion")
    ETargetLockPolicy GetTargetLockPolicy() { return TargetLockPolicy; }

    /////////////////////////////////////////////////////////////////////////
    // Minion Targeting.
    // There are essentially two types of targets
    // Waypoints, and then actually attackable things (heroes, towers, other minions)
    // GetTarget can return a waypoint actor OR an attackable target

    UFUNCTION(BlueprintCallable, Category = "AgoraMinion")
    virtual UObject* GetTarget() const;

    // Sets this minion's current target. Optionally can be specified to 
    UFUNCTION(BlueprintCallable, Category = "AgoraMinion")
    virtual void SetTarget(UObject* NewTarget, float TargetTimeoutDuration = -1.0f, bool bLockTargetForDuration = false);

    UFUNCTION(BlueprintCallable, Category = "AgoraMinion")
    virtual void ClearTarget();
	
protected:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AgoraMinion")
    FName TargetBlackboardKeyName = "Target";

    UPROPERTY(BlueprintReadOnly, Category = "AgoraMinion")
    ETargetLockPolicy TargetLockPolicy = ETargetLockPolicy::NotLocked;

    FTimerHandle TargetDropTimer;

};

