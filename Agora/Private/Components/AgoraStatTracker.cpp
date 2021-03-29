// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraStatTracker.h"
#include "GameFramework/Actor.h" // ????????????????????
#include "Agora.h"
#include "AgoraStatTrackingLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UAgoraStatTracker::UAgoraStatTracker()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
    bReplicates = true;
}

void UAgoraStatTracker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAgoraStatTracker, TrackedStats);

}

// Called when the game starts
void UAgoraStatTracker::BeginPlay()
{
	Super::BeginPlay();

    TrackedStats.OwningStatTracker = this;

}

bool UAgoraStatTracker::HasWriteAuthority()
{
    return GetOwner()->HasAuthority();
}

// Called every frame
void UAgoraStatTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAgoraStatTracker::Internal_NotifyStatChanged(const FString& StatName, float NewValue)
{
    GetStatChangedDelegate(StatName, true).Broadcast(StatName, NewValue);
}

void UAgoraStatTracker::ModifyStat(const FString& StatName, EStatModType ModifierType, float Value, bool CreateIfNoExist)
{
    if (!HasWriteAuthority()) { return; }

    FAgoraGameStat* Stat = nullptr;
    if (FindGameStat(StatName, Stat, CreateIfNoExist))
    {
        Stat->Modify(ModifierType, Value);
        TrackedStats.MarkItemDirty(*Stat);

        // Listen server will handle this onrep
        if (GetNetMode() == NM_DedicatedServer)
        {
            Stat->PostReplicatedChange(TrackedStats);
        }
        return;
    }

    return;
}

float UAgoraStatTracker::GetStatValue(const FString& StatName, bool CreateIfNoExist)
{
    FAgoraGameStat* Stat = nullptr;
    if (FindGameStat(StatName, Stat, CreateIfNoExist))
    {
        return Stat->Value;
    }

    return 0.0f;

}

void UAgoraStatTracker::CreateStatString(FString& OutString)
{
    
    OutString = "";

    OutString += FString::Printf(TEXT("%s Stat Output: \n"), *GetNameSafe(GetOwner()));

    for (FAgoraGameStat& Stat : TrackedStats.Items)
    {
        OutString += FString::Printf(TEXT("%s: %f \n"), *Stat.StatName, Stat.Value);
    }
}

FOnStatChangedDelegateSignature& UAgoraStatTracker::GetStatChangedDelegate(const FString& StatName, bool CreateIfNoExist)
{
    if (StatDelegates.Contains(StatName))
    {
        return StatDelegates[StatName];
    }

    return StatDelegates.Add(StatName, FOnStatChangedDelegateSignature());
}

bool UAgoraStatTracker::FindGameStat(const FString& StatName, FAgoraGameStat*& OutFoundStat, bool CreateIfNoExist)
{
    for (FAgoraGameStat& Stat : TrackedStats.Items)
    {
        if (Stat.StatName == StatName)
        {
            OutFoundStat = &Stat;
            return true;
        }
    }

    if (CreateIfNoExist && HasWriteAuthority())
    {
        FAgoraGameStat NewStat;
        NewStat.StatName = StatName;
        NewStat.Value = 0.0f;
        int32 NewStatIdx = TrackedStats.Items.Add(NewStat);
        OutFoundStat = &TrackedStats.Items[NewStatIdx];
        TrackedStats.MarkItemDirty(TrackedStats.Items[NewStatIdx]);
        TRACE(AgoraStatTracker, Log, "Created new stat %s for %s", *StatName, *GetNameSafe(GetOwner()));
        return true;
    }

    return false;
}

void FAgoraGameStat::Modify(EStatModType ModifierType, float ModValue)
{
    if (ModifierType == EStatModType::Set)
    {
        Value = ModValue;
    }
    else
    {
        Value += ModValue;
    }
    
}

void FAgoraGameStat::PostReplicatedAdd(const struct FAgoraGameStatArray& InArraySerializer)
{
    InArraySerializer.OwningStatTracker->Internal_NotifyStatChanged(StatName, Value);
}

void FAgoraGameStat::PostReplicatedChange(const struct FAgoraGameStatArray& InArraySerializer)
{
    InArraySerializer.OwningStatTracker->Internal_NotifyStatChanged(StatName, Value);
}
