// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AgoraEntityLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AgoraEntity, Log, All);
DECLARE_STATS_GROUP(TEXT("AgoraEntity"), STATGROUP_AgoraEntity, STATCAT_Advanced);

class UTexture;
class AAgoraCharacterBase;
class APawn;
class AActor;
struct FAgoraSweepQueryParams;

UENUM(BlueprintType)
enum EDescriptor
{
    Invalid,
    Hero,
    Core,
    Tower,
    Inhibitor,
    Minion,
    JungleCreep,
    Spectator
};

/**
 * An entity is generally things which are controlled and exist in the world. Structures, Heroes, Minions, etc
 * fall into this category. 
 */
UCLASS()
class AGORA_API UAgoraEntityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    /*
     * Returns the icon that represents this entity.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraEntityLibrary")
    static UTexture* GetDisplayIcon(AActor* Actor);

    /**
    * Wrapper for determining which type of GameplayTag.Descriptor.Unit.* the entity is, if any
    */
    UFUNCTION(BlueprintPure, Category = "AgoraEntityLibrary")
    static EDescriptor GetEntityType(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "AgoraEntityLibrary")
    static bool IsDead(AActor* Actor);

    /**
     * Returns the respawn time for this entity. Currently assumes the passed in entity is of type AAgoraHeroBase.
     * Returns false if a respawn time could not be determined.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraEntityLibrary")
    static bool GetRemainingRespawnDuration(AActor* Actor, float& OutRespawnTime);

    UFUNCTION(BlueprintPure, Category = "AgoraEntityLibrary")
    static bool GetRespawnTimestamp(AActor* Actor, float& OutRespawnTimestamp);

    /**
     * Returns the scene component which represents where the 'target' point is for an entity.
     * Will return the actor's root component if a UAgoraSceneTargetComponent does not exist.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraEntityLibrary")
    static USceneComponent* GetEntityTargetLocation(AActor* Actor);

    /**
     * Returns a mesh component from an entity.
     * Assumes the entity is a character, will return nullptr otherwise.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraEntityLibrary")
    static USkeletalMeshComponent* GetMesh(AActor* Actor);

    // Currently blocked out because FAgoraSweepParams is being a pain at 4am
    ///**
    // * Returns all pawns in the world @WorldContext which satisfy @QueryParams
    // * Expensive operation.
    // * @Param OutPawns - pawns found
    // * @Param ReferenceActor - used to help the query params determine team allegiance. To be refactored later
    // * @Param QueryParams - optional conditions which must be satisfied
    // */
    //UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "AgoraEntityLibrary")
    //static void GetAllPawns(UObject* WorldContextObject, TArray<APawn*>& OutPawns, AActor* ReferenceActor, const FAgoraSweepQueryParams& QueryParams);

    //Completely disables keyboard and mouse input for this pawn.
    UFUNCTION(BlueprintCallable, Category = "AgoraEntityLibrary")
    static void CompletelyDisableInput(APawn* PawnToDisable);

    //Re-enable keyboard and mouse input for this pawn.
    UFUNCTION(BlueprintCallable, Category = "AgoraEntityLibrary")
    static void EnableInput(APawn* PawnToEnable);

    //Disable movement for this pawn. If you call this function n times, you must call EnableMovement n times to undo it all.
    UFUNCTION(BlueprintCallable, Category = "AgoraEntityLibrary")
    static void DisableMovement(APawn* PawnToDisable);

    //Enable movement for this pawn. If you call this function n times, you must call DisableMovement n times to undo it all.
    UFUNCTION(BlueprintCallable, Category = "AgoraEntityLibrary")
    static void EnableMovement(APawn* PawnToDisable);

    //Enable mouse look for this pawn. If you call this function n times, you must call DisableMouseLook n times to undo it all.
    UFUNCTION(BlueprintCallable, Category = "AgoraEntityLibrary")
    static void EnableMouseLook(APawn* PawnToEnable);

    //Disable mouse look for this pawn. If you call this function n times, you must call EnableMouseLook n times to undo it all.
    UFUNCTION(BlueprintCallable, Category = "AgoraEntityLibrary")
    static void DisableMouseLook(APawn* PawnToDisable);

    //Disable turn-in-place for this character. Moving the camera will no longer trigger a turn-in-place event.
    UFUNCTION(BlueprintCallable, Category = "AgoraEntityLibrary")
    static void DisableTurnInPlace(AAgoraCharacterBase* Character);

    //Re-enable turn-in-place for this character.
    UFUNCTION(BlueprintCallable, Category = "AgoraEntityLibrary")
    static void EnableTurnInPlace(AAgoraCharacterBase* Character);

};
