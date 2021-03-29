// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Class.h"
#include "AgoraStatTracker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChangedDelegateSignature, const FString&, ChangedStatName, float, NewValue);

UENUM(BlueprintType)
enum class EStatModType : uint8
{
    Set,
    Add
};

/**
 * Struct that maps stat names to values.
 */
USTRUCT(BlueprintType)
struct FAgoraGameStat : public FFastArraySerializerItem
{

    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category = "AgoraStat")
    FString StatName;

    UPROPERTY(BlueprintReadOnly, Category = "AgoraStat")
    float Value;
    
    void Modify(EStatModType ModifierType, float ModValue);

    // FFastArraySerializerItem
    void PostReplicatedAdd(const struct FAgoraGameStatArray& InArraySerializer);
    void PostReplicatedChange(const struct FAgoraGameStatArray& InArraySerializer);
    // ~FFastArraySerializerItem

};

/**
 * Wraps an array of stats, allowing us to receive OnRep on a per-item basis. See documentation for FFastArraySerializers and FFastArraySerializerItems
 */
USTRUCT()
struct FAgoraGameStatArray : public FFastArraySerializer
{
    GENERATED_BODY()
    
    UPROPERTY()
    TArray<FAgoraGameStat> Items;

    /** Needed to control delegates referenced by observers. Items will call Internal_NotifyStatChanged based on this reference*/
    UPROPERTY()
    UAgoraStatTracker* OwningStatTracker = nullptr;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FAgoraGameStat, FAgoraGameStatArray>(Items, DeltaParms, *this);
    }

};

template<>
struct TStructOpsTypeTraits< FAgoraGameStatArray > : public TStructOpsTypeTraitsBase2<FAgoraGameStatArray>
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};

/**
 * Tracks the values of stats designated by FNames. Stats can be observed through their assigned multicast delegates.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGORA_API UAgoraStatTracker : public UActorComponent
{
	GENERATED_BODY()

public:	

	UAgoraStatTracker();

    // UActorComponent interface
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    // ~UActorComponent interface

    /**
     * Modifies a the stat @StatName. Can create if the stat doesn't exist.
     */
    UFUNCTION(BlueprintCallable, Category = "AgoraStat")
    void ModifyStat(const FString& StatName, EStatModType ModifierType, float Value, bool CreateIfNoExist);

    /**
     * Retrieves the stat @StatName. Can create if the stat doesn't exist.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraStat")
    float GetStatValue(const FString& StatName, bool CreateIfNoExist);

    /**
     * Creates a string listing all stats that this component knows about and their current values.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraStat")
    void CreateStatString(FString& OutString);

    /**
     * Returns the delegate that cooresponds with @StatName. Can create a new delegate if one has not already been created.
     * If CreateIfNoExist is false and we currently do not have a delegate, will return DeadStatDelegate. This delegate will never be fired.
     */
    FOnStatChangedDelegateSignature& GetStatChangedDelegate(const FString& StatName, bool CreateIfNoExist);

protected:
    
    // UActorComponent interface
    virtual void BeginPlay() override;
    // ~UActorComponent interface

    /**
     * Returns true if we are able to create/modify stats.
     */
    UFUNCTION()
    bool HasWriteAuthority();

    /**
     * Used by FAgoraGameStat to notify us that a stat was changed. Will fire the delegate cooresponding to the stat, or create one if the stat
     * did not already have an associated delegate.
     */
    UFUNCTION()
    void Internal_NotifyStatChanged(const FString& StatName, float NewValue);

    /**
     * All stats that we are currently tracking. This is an array.
     */
    UPROPERTY(Replicated)
    FAgoraGameStatArray TrackedStats;
    
    /**
     * Attempts to find the stat @StatName, pushing its address in @OutFoundStat if it exists and returning true. 
     * Can create the stat if it doesn't already exist.
     */
    bool FindGameStat(const FString& StatName, FAgoraGameStat*& OutFoundStat, bool CreateIfNoExist);

    /**
     * Maps Stats to delegates. Delegates are fired when stats are changed.
     */
    TMap<FString, FOnStatChangedDelegateSignature> StatDelegates;

    /**
     * Invalid delegate reference. This will never be fired and anything bound to this is done through error.
     * Objects are only bound to this if a client call tried to bind to a delegate and it didnt exist (and the component was told not to create a delegate for the not-found stat).
     */
    FOnStatChangedDelegateSignature DeadStatDelegate;

    friend struct FAgoraGameStat;

};
