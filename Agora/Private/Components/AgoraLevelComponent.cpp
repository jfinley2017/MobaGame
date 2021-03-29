#include "AgoraLevelComponent.h"
#include "Engine/Engine.h"
#include "Agora.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

UAgoraLevelComponent::UAgoraLevelComponent()
{
	SetIsReplicated(true);
}

void UAgoraLevelComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAgoraLevelComponent, CurrentLevel);
    DOREPLIFETIME(UAgoraLevelComponent, LastLevelTime);
}

void UAgoraLevelComponent::BeginPlay()
{
    Super::BeginPlay();

    ensureMsgf(LevelTable, TEXT("Level table not set on level component on actor %s"), *GetOwner()->GetName());

    TArray<FName> RowNames = LevelTable->GetRowNames();
    MaxLevel = RowNames.Num();
}

void UAgoraLevelComponent::SetLevel(uint8 NewLevel)
{
    if (!GetOwner()->HasAuthority()) { return; }

    LastLevelTime = UAgoraBlueprintFunctionLibrary::GetTimeSinceStartOfGame(this);
    CurrentLevel = NewLevel;
    NotifyLevelUp(CurrentLevel);
    OnRep_CurrentLevel();
}

uint8 UAgoraLevelComponent::GetCurrentLevel()
{
    return CurrentLevel;
}

float UAgoraLevelComponent::GetPercentageTowardsNextLevel()
{
    unimplemented();
    return 0.0f;
}

void UAgoraLevelComponent::NotifyLevelUp(uint8 NewLevel)
{
    ReceiveLevelUp(NewLevel);
}

void UAgoraLevelComponent::BroadcastLeveledUp()
{
    OnLeveledUp.Broadcast(GetOwner(), CurrentLevel, LastLevelTime);
}

FXpLevelUpData* UAgoraLevelComponent::GetTotalXpForNextLevel()
{
	return CurrentLevel == MaxLevel ? GetTotalForLevel<FXpLevelUpData>(MaxLevel) : GetTotalForLevel<FXpLevelUpData>(CurrentLevel);
}

FLaneSpawnLevel* UAgoraLevelComponent::GetTotalMinionCountForNextLevel()
{
	return CurrentLevel == MaxLevel ? GetTotalForLevel<FLaneSpawnLevel>(MaxLevel) : GetTotalForLevel<FLaneSpawnLevel>(CurrentLevel);
}

template <class T>
T* UAgoraLevelComponent::GetTotalForLevel(uint8 SpecifiedLevel)
{
    if (!LevelTable) { return nullptr; }

    FString WhatIsThisArgumentFor;
    T* FoundRow = LevelTable->FindRow<T>(*FString::FromInt(SpecifiedLevel), WhatIsThisArgumentFor);

    // if for some reason you want to have levels beyond the range of your tables then feel free to remove this
    ensureMsgf(FoundRow, TEXT("Failed to find row for level component"));
    return FoundRow;
}

void UAgoraLevelComponent::OnRep_CurrentLevel()
{
    BroadcastLeveledUp();
}
