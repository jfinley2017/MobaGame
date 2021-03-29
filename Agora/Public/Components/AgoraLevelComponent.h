#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Agora.h"
#include "AgoraLevelComponent.generated.h"


//THE FOLLOWING STRUCTS DON'T BELONG HERE BUT THEY HAVE TO BE HERE FOR THE LINUX BUILD TO WORK
//DON'T MOVE THESE. PLEASE. I'LL COME BACK TO IT SOON.
//THIS WORKS FOR NOW. JUST LET IT BE. PLEASE AND THANK YOU.
// -paca

/**
 * Broadcasts when the character levels.
 * Including a timestamp due to cases where level reps late, we wouldn't want to play effects for that but we would still
 * like to update the level. 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLevelUpSignature, AActor*, LeveledActor, int32, NewLevel, float, LastLevelUpTimestamp);

// Extend this struct with the properties you want to scale by level
USTRUCT(BlueprintType)
struct FXpLevelUpData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FXpLevelUpData()
		: XpToLevel(0)
	{}

	// This is the *total* amount of xp required to the level
	// TODO: Try overriding OnPostDataImport to automatically calculate this from the difference between levels
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelUp)
		int32 XpToLevel;
};

USTRUCT(BlueprintType)
struct FTimeLevelUpData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FTimeLevelUpData()
		: SecondsToLevelUp(0)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelUp)
		int32 SecondsToLevelUp;
};

// Controls how many minions should be spawned based on time
USTRUCT(BlueprintType)
struct FLaneSpawnLevel : public FTimeLevelUpData
{
	GENERATED_USTRUCT_BODY()

public:

	FLaneSpawnLevel()
		: NumMeleeMinions(0)
		, NumRangedMinions(0)
		, NumSiegeMinions(0)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelUp)
		uint8 NumMeleeMinions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelUp)
		uint8 NumRangedMinions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelUp)
		uint8 NumSiegeMinions;
};


UCLASS( ClassGroup=(Agora))
class AGORA_API UAgoraLevelComponent : public UActorComponent
{
	GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FOnLevelUpSignature OnLeveledUp;

	UAgoraLevelComponent();

    /**
     * Levels the component up to the passed in level. Useful for testing, might be worth moving this into a protected state
     * (or, perhaps we should consider this a cheat function and have a different protected function for traditional levelups). 
     * Most levelup scenarios can be determined by watching external state or otherwise deriving a levelup internally. We don't need external code telling us we leveled up, basically.
     */
    UFUNCTION()
    virtual void SetLevel(uint8 NewLevel);

    UFUNCTION(BlueprintPure, Category = "AgoraLevelComponent")
    uint8 GetCurrentLevel();

    /** Returns how far along we are towards the next level, 0 - 1.0 */
    UFUNCTION(BlueprintPure, Category = "AgoraLevelComponent")
    virtual float GetPercentageTowardsNextLevel();

	FXpLevelUpData* GetTotalXpForNextLevel();
	FLaneSpawnLevel* GetTotalMinionCountForNextLevel();

    template <class T>
    T* GetTotalForLevel(uint8 SpecifiedLevel);

protected:

    virtual void BeginPlay() override;

    /** Notifys ourselves that we leveled up */
    virtual void NotifyLevelUp(uint8 NewLevel);

    /** Blueprint/Native hook ReceiveLevelUp event */
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraLevelComponent")
    void ReceiveLevelUp(uint8 NewLevel);

    /** Notifys observers that this component leveled up */
    virtual void BroadcastLeveledUp();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LevelComponent")
    UDataTable* LevelTable;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentLevel)
    uint8 CurrentLevel = 1;

    UPROPERTY(BlueprintReadOnly)
    uint8 MaxLevel = 1;
  
    /** Is set based on time since the start of the game, set because we don't know when CurrentLevel will be replicated again (and we don't want to play effects if it happened a while ago) */
    UPROPERTY(BlueprintReadOnly, Replicated)
    float LastLevelTime = -999.0f;

    UFUNCTION()
    void OnRep_CurrentLevel();
};
