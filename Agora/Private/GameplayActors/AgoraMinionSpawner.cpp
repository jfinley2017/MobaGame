#include "AgoraMinionSpawner.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "TeamInterface.h"
#include "AgoraTimeLevelComp.h"
#include "GameFramework/GameMode.h"
#include "Modes/AgoraGameState.h"
#include "Modes/AgoraGameMode.h"

AAgoraMinionSpawner::AAgoraMinionSpawner()
{
	LevelComp = CreateDefaultSubobject<UAgoraTimeLevelComp>(TEXT("LevelComp"));

	bNetLoadOnClient = false;
	bOnlyRelevantToOwner = true;
}
	
void AAgoraMinionSpawner::BeginPlay()
{
    Super::BeginPlay();
	check(HasAuthority());

	GetWorld()->GetGameState<AAgoraGameState>()->OnMatchStateChanged.AddDynamic(this, &AAgoraMinionSpawner::HandleMatchStateChange);

	LevelComp->OnLeveledUp.AddDynamic(this, &AAgoraMinionSpawner::NotifyLeveledUp);
}

void AAgoraMinionSpawner::HandleMatchStateChange(FName State)
{
	if (State == MatchState::GameBegun)
	{
		FTimerHandle ResetMinionWaveTaskTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(ResetMinionWaveTaskTimerHandle, this, &AAgoraMinionSpawner::SpawnWave, DelayBetweenWaves, true, FirstSpawnDelay);
	}
}

void AAgoraMinionSpawner::NotifyLeveledUp(AActor* LeveledActor, int32 NewLevel, float LastLevelTime)
{
    ReceiveLeveledUp(LeveledActor, NewLevel, LastLevelTime);
}

void AAgoraMinionSpawner::SpawnWave()
{

}

// Begin TeamInterface
ETeam AAgoraMinionSpawner::GetTeam() const
{
    return Team;

}

void AAgoraMinionSpawner::SetTeam(ETeam NewTeam)
{
    Team = NewTeam;
    OnTeamChanged.Broadcast(this);
}

FOnTeamChangedSignature& AAgoraMinionSpawner::GetTeamChangedDelegate()
{
    return OnTeamChanged;
}
// ~TeamInterface