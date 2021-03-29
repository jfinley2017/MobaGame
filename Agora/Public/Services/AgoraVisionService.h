// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Class.h"
#include "Net/UnrealNetwork.h"

#include "Interfaces/TeamInterface.h"
#include "AgoraVisionService.generated.h"

/**
 * Thoughts on vision.
 * Definitions:
 * 
 * Revealed - A unit on an opposing team has vision on me. This does NOT mean that every local player has vision on me however (see below).
 * -- Think in the case of a neutral creep. Team1 may see the neutral, but Team2 does not. This neutral creep is still classified as Revealed regardless.
 *
 * Local player. The screen currently viewing me. If there are 5 clients in a dedicated server then there are 5 local players. Local players can be 
 * -- different things, eg a player on a team versus a spectator. Need to also handle the cases where a spectator wants to mimic the vision of certain teams.
 * -- Using the viewtarget's team and recalculating when the view target changes might be a good solution to this. 
 */


 /**
 * Struct that maps stat names to values.
 */
USTRUCT(BlueprintType)
struct FAgoraVisibleActor : public FFastArraySerializerItem
{

    GENERATED_BODY()

    UPROPERTY()
    AActor* Actor;

    // FFastArraySerializerItem
    void PostReplicatedAdd(const struct FAgoraVisibleActorsArray& InArraySerializer);
    void PreReplicatedRemove(const struct FAgoraVisibleActorsArray& InArraySerializer);
    // ~FFastArraySerializerItem

};

/**
 * Wraps an array of stats, allowing us to receive OnRep on a per-item basis. See documentation for FFastArraySerializers and FFastArraySerializerItems
 */
USTRUCT()
struct FAgoraVisibleActorsArray : public FFastArraySerializer
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FAgoraVisibleActor> Items;


    bool Contains(const AActor* Actor);
    void Remove(const AActor* Actor);
    void AddUnique(AActor* Actor);
   

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FAgoraVisibleActor, FAgoraVisibleActorsArray>(Items, DeltaParms, *this);
    }

};

template<>
struct TStructOpsTypeTraits< FAgoraVisibleActorsArray > : public TStructOpsTypeTraitsBase2<FAgoraVisibleActorsArray>
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};




/**
 * Responsible for keeping track of a team's vision. 
 */
UCLASS()
class AGORA_API AAgoraVisionService : public AInfo
{
	GENERATED_BODY()
    	
public:

    AAgoraVisionService();

    /**
     * Notifys the service that an actor @Spotter has obtained vision on @Subject for @SpottingTeam.
     */
    UFUNCTION()
    void NotifyGainedVision(AActor* Spotter, ETeam SpottingTeam, AActor* Subject);

    /**
     * Notifys the service that an actor @Spotter has lost vision on @Subject for @SpottingTeam.
     */
    UFUNCTION()
    void NotifyLostVision(AActor* Spotter, ETeam SpottingTeam, AActor* Subject);
    
    /**
     * Returns true if the team specified by @Team has vision on the subject. 
     * Returns false if @Team is not recognized.
     */
    UFUNCTION()
    bool IsVisibleForTeam(ETeam Team, const AActor* Subject);

    /**
     * Returns true if the viewing actor can see the subject.
     * Requires both Viewer and Subject to implement ITeamInterface, returns true if one does not implement ITeamInterface
     */
    UFUNCTION()
    bool IsVisibleFor(const AActor* Viewer, const AActor* Subject);

    /**
     * Notifys the service that the local player's team has changed and we need to recalculate visibility on all actors.
     */
    UFUNCTION()
    void NotifyLocalPlayerTeamChanged(AActor* LocalPlayer);

    UFUNCTION()
    void GenerateDebugString(FString& OutDebugString);

protected:

    // AActor interface
    virtual void BeginPlay() override;
    // ~AActor interface

    /**
     * Recalculates the local vision state on all actors. **EXPENSIVE**
     */
    UFUNCTION()
    void RecalculateVisiblityForAll();

    UFUNCTION()
    void SetupLocalPlayer();

    /**
     * Maps Spotted->[Spotters]. If a spotted actor has nothing spotting it, it is not visible anymore for this team and should be removed.
     */
    TMap<AActor*, TArray<AActor*> > DawnViewingReferences;

    /**
     * The keys of ViewingReferences. Replicated so that clients can do their own vision checks.
     */
    UPROPERTY(Replicated)
    FAgoraVisibleActorsArray DawnSpottedActors;


    TMap<AActor*, TArray<AActor*> > DuskViewingReferences;
    UPROPERTY(Replicated)
    FAgoraVisibleActorsArray DuskSpottedActors;


    
};
