// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayAbilities/Public/GameplayEffectTypes.h"

#include "TeamInterface.h"
#include "Runtime/GameplayTags/Classes/GameplayTagAssetInterface.h"
#include "AgoraTypes.h"
#include "AgoraCharacterBase.h"
#include "AgoraGameplayTags.h" // For EDescriptor
#include "AgoraEntityLibrary.h"
#include "AgoraSweepLibrary.h"
#include "Options/AgoraUserSettings.h"

#include "AgoraBlueprintFunctionLibrary.generated.h"

class AActor;
class UInterface;
class UShapeComponent;
class UAnimMontage;
class UAbilitySystemComponent;
class AAgoraWorldSettings;
class UAgoraDataSingleton;
class UTexture2D;
class UAgoraSceneTargetComponent;
class USkeletalMeshComponent;
class UTexture;



USTRUCT(BlueprintType)
struct FSetByCallerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "SetByCaller")
	FGameplayTag DataTag;
	
	UPROPERTY(BlueprintReadWrite, Category = "SetByCaller")
	float Magnitude;
};

UENUM(BlueprintType)
enum EClassCheckEnum 
{
    OnSuccess,
    OnFailure
};

UENUM(BlueprintType)
enum EClampType
{
    ClampNone,
    ClampMin,
    ClampMax,
    ClampBoth
};

USTRUCT(BlueprintType)
struct FAgoraNetConnection
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category = "AgoraNetConnection")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "AgoraNetConnection")
    float Ping = 0.0f;

    // Percentage
    UPROPERTY(BlueprintReadOnly, Category = "AgoraNetConnection")
    float PktLoss = 0.0f;

};


/**
 * 
 */
UCLASS()
class AGORA_API UAgoraBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    //////////////////////////////////////////////////////////////////////////
    // General Utility

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static UAgoraUserSettings* GetAgoraUserSettings();

    UFUNCTION(BlueprintPure, Category = "Agora")
    static bool GetPlayerNetConnectionDetails(const APlayerController* PlayerController, FAgoraNetConnection& OutNetConnection);

    UFUNCTION(BlueprintPure, Category = "Agora")
    static bool IsRunningWithEditor();
    
    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "Agora")
    static APlayerController* GetViewingPlayerController(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "Agora|GameMode")
	static AAgoraWorldSettings* GetAgoraWorldSettings(UObject* WorldContextObject);

    /**
     * Returns the Data singleton.
     */
    UFUNCTION(BlueprintPure, Category = "Agora")
    static UAgoraDataSingleton* GetGlobals();

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static UObject* BP_GetDefaultObject(UClass* ObjClass);

    /**
     * Returns true if we are not authority on a ENetMode::DedicatedServer netmode
     */
    UFUNCTION(BlueprintPure, Category = "Agora")
    static bool ShouldPlayCosmetics(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static FGameplayAttribute GetAttributeFromString(const FString& AttributeName, UAbilitySystemComponent* ASC);
    
    /**
     * Returns the time since the game started. 
     * Useful for determining when things should ACTUALLY be done, without constantly having to query for a 'warmup' time offset
     * EG. If there is a 45s time between minions spawning and one wants minions to spawn every 30s, you wouldn't be able to simply use GetTimeSeconds to determine 
     * whether or not to spawn the camp.
     * @Return time since the game started. CAN BE NEGATIVE.
     */
    UFUNCTION(BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Agora")
    static float GetTimeSinceStartOfGame(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", WorldContext = "WorldContextObject"), Category = "Agora")
	static AAgoraWorldSettings* GetWorldSettings(UObject* WorldContextObject);

    // comment me
    UFUNCTION(BlueprintCallable, Category = "Agora")
    static float CalculateMontagePlayRate(UAnimMontage* MontageToScale, float DesiredPlayLength, EClampType ClampMode, float MinPlayRate, float MaxPlayRate);

    // comment me
    UFUNCTION(BlueprintPure, Category = "Agora")
    static FHitResult DrawTraceFromActor(AActor* InSourceActor, float MaxRange, bool bDebug);

    /**
     * Returns all pawns in the world @WorldContext which satisfy @QueryParams
     * Expensive operation.
     * @Param OutPawns - pawns found
     * @Param ReferenceActor - used to help the query params determine team allegiance. To be refactored later
     * @Param QueryParams - optional conditions which must be satisfied
     */
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "Agora")
    static void GetAllPawns(UObject* WorldContextObject, TArray<APawn*>& OutPawns, AActor* ReferenceActor, const FAgoraSweepQueryParams& QueryParams);

	//Applies an impulse to this character's CMC using the specified Up and Forward magnitudes.
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static void AddImpulseToCharacterWithMagnitudes(AAgoraCharacterBase* Character, float UpMagnitude, float ForwardMagnitude);

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static void ImpulseActors(const TArray<AActor*>& Actors, const FVector& Impulse);

	//Applies the given impulse vector to the character's CMC.
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static void AddImpulseToCharacterWithVector(AAgoraCharacterBase* Character, FVector Impulse);
	
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static void DisableGravityForCharacter(AAgoraCharacterBase* Character);

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static void EnableGravityForCharacter(AAgoraCharacterBase* Character);

    // comment me
    UFUNCTION(BlueprintPure, Category = "Agora")
    static float GetSectionLengthFromIndex(UAnimMontage* Montage, int32 SectionIndex = 0);

    // comment me
    UFUNCTION(BlueprintPure, Category = "Agora")
    static float GetSectionLengthFromName(UAnimMontage* Montage, FName SectionName = NAME_None);

    /**
     * Generates a random float between Min and Max. This is here because I (Arsenic) am not sure
     * whether or not we're going to need to do our own custom seeding. If every random use case comes 
     * through here then the change in either case will be minimal
     * @Param Min minimum possible value
     * @Param Max maximum possible value
     * @Ret a float between @Min and @Max, inclusive
     */
    UFUNCTION(BlueprintPure, Category = "Agora")
    static float GenerateRandomFloat(float Min, float Max);

	//Generates the options end of an Open Level URL given a map of options
	UFUNCTION(BlueprintPure, Category = "Agora")
	static FString CreateOptionsStringForOpenLevel(const TMap<FString, FString>& Options);

    //--------------------------------------------------------------------------
    // Gameplay

    /**
     * Determines the damage type from a GameplayEffectContextHandle
     * @Param InHandle handle to query for damage type
     * @Return EDamageType specify the damage type, EDamageType::DMGInvalid for none.
     */
    UFUNCTION(BlueprintPure, Category = "Damage")
    static EDamageType DetermineDamageTypeFromContextHandle(const FGameplayEffectContextHandle& InHandle);

    /**
     * Determines the direction that a hit occurred as a string "Front", "Left", "Right", "Back" or ""
     * @Param RightVector The right vector of the actor which was hit
     * @Param HitResultNormal The normal of the hit 
     * @Return "Front", "Left", "Right", "Back" or ""
     */
    UFUNCTION(BlueprintPure, Category = "Damage")
    static FName DetermineHitDirection(const FVector& RightVector, const FVector& HitResultNormal);

	/** Retrieves the surface type from the PhysMaterialOverride PM from the static mesh actor (if it exists) inside the given FHitResult.
	This is a workaround to get past the fact that setting a PM on a mesh sets it on the collision, not the material, which is super mcBad.
	The PM on the mesh's collision is normally not given to us in an FHitResult, because it's looking for a PM attached to the material instead.
	This fixes that.
	You're welcome Tim.
	*/
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static EPhysicalSurface GetStaticMeshSurfaceTypeFromHitResult(const FHitResult& HitResult);

	/**
	Similar to GetStaticMeshSurfaceTypeFromHitResult(), but for ALandscapes.
	*/
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static EPhysicalSurface GetLandscapeSurfaceTypeFromHitResult(const FHitResult& HitResult);
	
	//Exposes the AActor::UpdateOverlaps() function to BP. Effectively retriggers Overlap events for a given actor.
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static void UpdateActorOverlaps(AActor* Actor);

	//Exposes the UPrimitiveComponent::UpdateOverlaps() function to BP. Effectively retriggers Overlap events for a given component.
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static void UpdateComponentOverlaps(UPrimitiveComponent* Component);
	
	//Returns a vector corresponding to the point the player is looking at. Defaults to a 10000 unit range.
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static FVector GetPlayerLookingPoint(AAgoraCharacterBase* Player, float Range = 10000.0f);

	//Gets the location and rotation of the player's view point
	UFUNCTION(BlueprintPure, Category = "Agora")
	static void GetPlayerViewLocationAndRotation(AActor* Player, FVector& OutLocation, FRotator& OutRotation);

	//Saves the specified string to the specified file, inside the Saved directory.
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static void SaveTextFile(const FString& FileName, const FString& FileContents);

	//Loads the contents of the specified file, inside the Saved directory.
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static bool LoadTextFile(const FString& FileName, FString& OutContents);

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static bool DeleteTextFile(const FString& FileName);

    //------------------------------------------------------------------------
    // Interface Utility

    /**
     * Determines whether or not a UObject IsA Other. Optionally logs/ensures errors.
     * @Param TestActor the actor to test for the class @Other
     * @Param Other the class to test @TestActor against
     * @Param bShouldLogFailure should we print when this fails?
     * @Param bShouldEnsureTrue should we ensure this test passes?
     * @Param Branches blueprint exec paths
     */
    UFUNCTION(BlueprintCallable, Category = "Agora", meta = (ExpandEnumAsExecs = "Branches"))
    static void IsALogging(UObject* TestActor, UClass* Other, bool bShouldLogFailure, bool bShouldEnsureTrue, TEnumAsByte<EClassCheckEnum>& Branches);


    /**
    * Determines whether or not a UObject IsA Other. Optionally logs/ensures errors.
    * @Param TestActor the actor to test for the interface @Interface
    * @Param Interface the class to test @TestActor against
    * @Param bShouldLogFailure should we print when this fails?
    * @Param bShouldEnsureTrue should we ensure this test passes?
    * @Branches blueprint exec paths
    */
    UFUNCTION(BlueprintCallable, Category = "Agora", meta = (ExpandEnumAsExecs = "Branches"))
    static void ImplementsInterfaceLogging(UObject* TestActor, TSubclassOf<UInterface> Interface, bool bShouldLogFailure, bool bShouldEnsureTrue, TEnumAsByte<EClassCheckEnum>& Branches);

	UFUNCTION(BlueprintCallable, Category = "Utility", meta = (AdvancedDisplay = 2))
	static FGameplayCueParameters MakeTargetedGameplayCueParams(UAbilitySystemComponent* AbilitySystemComp, AActor* Target, AActor* Instigator = nullptr, FVector Origin = FVector::ZeroVector);

	//--------------------------------------------------------------------------
	// Vector utilities
	//--------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static FVector GetPositionInFrontOfActor(AActor* Actor, float DistanceInFront);

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static FVector GetPositionAboveActor(AActor* Actor, float DistanceAbove);

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static FVector GetVectorPointingFromActor(AActor* Actor, float UpMagnitude, float ForwardMagnitude);

	UFUNCTION(BlueprintCallable, Category = "Agora")
	static FVector ScaleVectorToLength(FVector Vector, float Length);

    //////////////////////////////////////////////////////////////////////////
    // Gameplay Cues

    /**
     * Plays a local cue @CueTag for @CueTarget.
     * @Param CueTarget the actor to play the cue for
     * @Param CueTag the tag corresponding to the gameplay cue(s) to fire.
     * @Param EventData the data to be sent along to any cues fired
     */
    UFUNCTION(BlueprintCallable, Category = "Agora")
    static void PlayLocalGameplayCueForActor(AActor* CueTarget, FGameplayTag CueTag, const FGameplayCueParameters& EventData);

    //Might make this more generalized later. For now, tries to get a scenecomp from the first actor in the target data object. may return null.
	//This is just here so I don't have to do convoluted BP spaghetti null checks and other such crap.
	UFUNCTION(BlueprintCallable, Category = "Agora")
	static UAgoraSceneTargetComponent* GetSceneComponentFromFirstActorInTargetData(FGameplayAbilityTargetDataHandle TargetData);
	
	//-------------------------------------------------------------------------
    // GameplayEvents

    // Send a gameplay event to an actor
    // @RetVal -1 if failed to send gameplay event
    UFUNCTION(BlueprintCallable, Category = "Agora")
    static int32 SendGameplayEventToActor(AActor* Actor, const FGameplayTag EventTag, const FGameplayEventData Payload);

    //-------------------------------------------------------------------------
    // Gameplay Effect Context 

    /**
     * Returns the GameplayTags that a FGameplayEffectDamageContext holds.
     * Ensures that @InHandle is valid.
     * @Param InHandle ContextHandle which holds the tags
     * @Param OutContainer Container which will have @InHandle's tags appended to it on success
     */
    UFUNCTION(BlueprintPure, Category = "EffectContext")
    static void GetGameplayEffectContextTags(const FGameplayEffectContextHandle& InHandle, FGameplayTagContainer& OutContainer);

    // comment me
    UFUNCTION(BlueprintPure, Category = "EffectContext")
    static FGameplayTagContainer GetSourceAbilityTags(const FGameplayEffectContextHandle& InHandle);

    // comment me
    UFUNCTION(BlueprintPure, Category = "EffectContext")
    static AActor* GetEffectCauser(const FGameplayEffectContextHandle& InHandle);

    // comment me
    UFUNCTION(BlueprintCallable, Category = "Agora")
    static void ApplySetByCallerGameplayEffectToTarget(UAbilitySystemComponent* TargetComp, int32 Level, TSubclassOf<UGameplayEffect> GameplayEffectClass, TArray<FSetByCallerData> SetByCallerData);


    //////////////////////////////////////////////////////////////////////////
    // Deprecated
    



    /** Returns true if ActorOne has vision of ActorTwo */
    UFUNCTION(BlueprintPure, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraTeamLibrary::IsFriendly instead."), Category = "Agora")
        static bool HasVisionOf(AActor* ActorOne, AActor* ActorTwo);

    // comment me
    UFUNCTION(BlueprintPure, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraSweepLibrary::GenerateMeleeHitResult_Cleave instead."), Category = "Agora")
    static FMeleeHitResult GenerateMeleeHitResult_Cleave(AActor* PerformingActor, FVector HitBox, float MaxRange, bool bDebug);


     /**
    * This sweep is not like the others!
    * This function randomly shotguns line traces from the specified origin onto a rectangle located at (PyramidOrigin + PyramidHeight) with the specified dimensions.
    * Useful for things like Murdock's buckshot, where a pyramidal collision check is ideal.
    * Uses a caller-controllable bivariate normal distribution to randomly distribute the line traces. Defaults to an evenly distributed spread of points.
    * @Param SweepingActor The actor performing the Sweep
    * @Param AgoraQueryParams Should we ignore Enemies? Friendlies?
    * @Param BaseWidth Width of the base of the pyramid
    * @Param BaseHeight Height of the base of the pyramid
    * @Param PyramidOrigin The location of the "tip" of the pyramid
    * @Param PyramidHeight The height of the pyramid from tip to base.
    * @Param PyramidRotation The rotation of the pyramid.
    * @Param OutOverlappingActors Filled found actors that pass additional filtering
    * @Param IgnoredActors Actors which we should not consider
    * @Param Spread Two values ranging from [1.0, 0.0) describing the spread of the traces. X value controls the spread along the width-axis. Y value controls the spread along the height-axis. E.g. A spread of (1.0, 1.0) means the traces are evenly distributed across the base. A spread of (0.1, 0.1) will make the traces concentrated more in the center.
    */
    UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"), Category = "Agora")
        static bool AgoraSweepMultiPyramid(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, const float BaseWidth, const float BaseHeight,
                                           const FVector& PyramidOrigin, const float PyramidHeight, const FRotator& PyramidRotation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors, const FVector2D& Spread = FVector2D(1.0f, 1.0f), bool bDebug = false);

    //--------------------------------------------------------------------------
    // Stuff related to collisions and target actor stuff

    /**
    * Performs an AgoraSweepMulti in a sphere around @SweepLocation
    * Filters for team relationships to @SweepingActor.
    * @Param SweepingActor The actor performing the sweep
    * @Param AgoraQueryParams Should we ignore Enemies? Friendlies?
    * @Param SweepSphereRadius The radius extending from @SweepStartLoc
    * @Param SweepLocation The location at which we are sweeping
    * @Param OutOverlappingActors Filled found actors that pass additional filtering
    * @Param IgnoredActors Actors which we should not consider
    * @Return true if the sweep did not fail.
    */
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraSweepLibrary::AgoraSweepMultiSphere instead."), meta = (WorldContext = "WorldContextObject"), Category = "Agora")
        static bool AgoraSweepMultiSphere(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, float SweepSphereRadius, const FVector& SweepLocation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors);

    /**
    * Performs an AgoraSweepMulti in a box around @SweepLocation
    * Filters for team relationships to @SweepingActor.
    * @Param SweepingActor The actor performing the Sweep
    * @Param AgoraQueryParams Should we ignore Enemies? Friendlies?
    * @Param SweepBoxExtent The extent of the sweeping box.
    * @Param SweepLocation The location at which we are sweeping
    * @Param SweepRotation The rotation at which are we sweeping
    * @Param OutOverlappingActors Filled found actors that pass additional filtering
    * @Param IgnoredActors Actors which we should not consider
    * @Return true if the sweep did not fail.
    */
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraSweepLibrary::AgoraSweepMultiBox instead."), meta = (WorldContext = "WorldContextObject"), Category = "Agora")
        static bool AgoraSweepMultiBox(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, const FVector& SweepBoxExtent, const FVector& SweepLocation, const FRotator& SweepRotation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors);


    /**
    * Performs an AgoraSweepMulti in a capsule around @SweepLocation
    * Filters for team relationships to @SweepingActor.
    * @Param SweepingActor The actor performing the Sweep
    * @Param AgoraQueryParams Should we ignore Enemies? Friendlies?
    * @Param SweepCapsuleRadius The radius of the sweeping capsule.
    * @Param SweepCapsuleHalfHeight The half-height of the sweeping capsule.
    * @Param SweepLocation The location at which we are sweeping
    * @Param SweepRotation The rotation at which are we sweeping
    * @Param OutOverlappingActors Filled found actors that pass additional filtering
    * @Param IgnoredActors Actors which we should not consider
    * @Return true if the sweep did not fail.
    */
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraSweepLibrary::AgoraSweepMultiCapsule instead."), meta = (WorldContext = "WorldContextObject"), Category = "Agora")
        static bool AgoraSweepMultiCapsule(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, const float SweepCapsuleRadius, const float SweepCapsuleHalfHeight, const FVector& SweepLocation, const FRotator& SweepRotation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors);



     /**
     * Determines if two actors are on the same team.
     * Assumes both actors implement ITeamInterface
     * @Ret true if Actor1 and Actor2 are friendly.
     */
    UFUNCTION(BlueprintPure, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraTeamLibrary::IsFriendly instead."), Category = "TeamInterface")
        static bool IsFriendly(const AActor* Actor1, const AActor* Actor2);

    /**
     * Wrapper for determining which type of GameplayTag.Descriptor.Unit.* the actor is, if any
     */
    UFUNCTION(BlueprintPure, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::GetEntityType instead."), Category = "Agora")
        static EDescriptor GetDescriptor(AActor* Actor);

    UFUNCTION(BlueprintPure, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::GetMesh instead."), Category = "Agora")
        static USkeletalMeshComponent* GetMeshFromActor(AActor*Actor);

    UFUNCTION(BlueprintPure, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::GetDisplayIcon instead."), Category = "Agora")
        static UTexture* GetDisplayIcon(AActor* Actor);

    //Completely disables keyboard and mouse input for this pawn.
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary:CompletelyDisableInput instead."), Category = "Agora")
        static void CompletelyDisableInput(APawn* PawnToDisable);

    //Re-enable keyboard and mouse input for this pawn.
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::EnableInput instead."), Category = "Agora")
        static void EnableInput(APawn* PawnToEnable);

    //Disable movement for this pawn. If you call this function n times, you must call EnableMovement n times to undo it all.
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::DisableMovement instead."), Category = "Agora")
        static void DisableMovement(APawn* PawnToDisable);

    //Enable movement for this pawn. If you call this function n times, you must call DisableMovement n times to undo it all.
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::EnableMovement instead."), Category = "Agora")
        static void EnableMovement(APawn* PawnToDisable);

    //Enable mouse look for this pawn. If you call this function n times, you must call DisableMouseLook n times to undo it all.
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::EnableMouseLook instead."), Category = "Agora")
        static void EnableMouseLook(APawn* PawnToEnable);

    //Disable mouse look for this pawn. If you call this function n times, you must call EnableMouseLook n times to undo it all.
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::DisableMouseLook instead."), Category = "Agora")
        static void DisableMouseLook(APawn* PawnToDisable);

    //Disable turn-in-place for this character. Moving the camera will no longer trigger a turn-in-place event.
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::DisableTurnInPlace instead."), Category = "Agora")
        static void DisableTurnInPlace(AAgoraCharacterBase* Character);

    //Re-enable turn-in-place for this character.
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraEntityLibrary::EnableTurnInPlace instead."), Category = "Agora")
        static void EnableTurnInPlace(AAgoraCharacterBase* Character);


    /**
     * Returns a gameplay tag with the name @Tag.
     * Acts as an ensure if no GameplayTag with the name @Tag is found.
     * @Param Tag the name of the tag in 'X.Y.Z' format
     * @Ret the retrieved tag.
     */
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraTagLibrary::GetGameplayTag."), Category = "Agora")
        static FGameplayTag GetGameplayTag(const FName Tag);

    UFUNCTION(BlueprintPure, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraTagLibrary::ArrayOfTagsToString."), Category = "Agora")
        static FString ArrayOfTagsToString(TArray<FGameplayTag> Tags);

    // Check if the asset has a gameplay tag that matches against the specified tag (expands to include parents of asset tags)
    // @Return false if actor doesn't implement IGameplayTagAssetInterface
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraTagLibrary::HasMatchingGameplayTag."), Category = "Agora")
        static bool HasMatchingGameplayTag(AActor* Actor, const FGameplayTag Tag);

    // Check if the asset has gameplay tags that matches against all of the specified tags(expands to include parents of asset tags)
    // @Return false if actor doesn't implement IGameplayTagAssetInterface
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraTagLibrary::HasAllMatchingGameplayTags."), Category = "Agora")
        static bool HasAllMatchingGameplayTags(AActor* Actor, const FGameplayTagContainer& TagContainer);

    // Check if the asset has gameplay tags that matches against any of the specified tags(expands to include parents of asset tags)
    // @Return false if actor doesn't implement IGameplayTagAssetInterface
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraTagLibrary::HasAnyMatchingGameplayTags."), Category = "Agora")
        static bool HasAnyMatchingGameplaytags(AActor* Actor, const FGameplayTagContainer& TagContainer);

    // Returns the actors owned GameplayTags
    // Does nothing if Actor does not own any GameplayTags. Does nothing if Actor does not implement IGameplayAssetInterface
    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction,
    DeprecationMessage = "Use AgoraTagLibrary::GetOwnedGameplayTags."), Category = "Agora")
        static void GetOwnedGameplayTags(AActor* Actor, FGameplayTagContainer& OutContainer);


};