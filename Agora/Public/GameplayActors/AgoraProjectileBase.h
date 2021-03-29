// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilities/Public/GameplayEffectTypes.h"
#include "AgoraTypes.h"
#include "AgoraProjectileBase.generated.h"

class AActor;
class IDamageReceiverInterface;

DECLARE_LOG_CATEGORY_EXTERN(AgoraProjectile, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeliverPayloadSignature, AActor*, Target);

USTRUCT(BlueprintType)
struct FProjectileParams
{
    GENERATED_BODY();

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    float MaxRange = -1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    float InitialSpeed = 3000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    float MaxSpeed = 3000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    float CollisionRadius = 5.0f;

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    bool bIsHomingProjectile = false;

    // If this is set, then bIsHoming is also set in PMC
    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    TWeakObjectPtr<USceneComponent> HomingTarget_Component;
    
    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    AActor* HomingTarget;

    // Whether or not we damage friendlies
    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    bool bAllowFriendlyFire = false;

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    bool bAllowSelfHit = false;

    // Whether or not we pass through everything but targets
    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    bool bIgnoreAllButTarget = false;

    // Whether or not we pass through terrain (WorldStatic)
    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    bool bPassesThroughTerrain = false;

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    bool bPassesThroughFriendlyMinions;

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    bool bPassesThroughFriendlyTowers;

    //////////////////////////////////////////////////////////////////////////
    // Visualization
    // These cues are played LOCALLY, as the projectile itself is replicated to all clients.
    // This saves multicast calls, which can be sparse
    // Also might allow us to plug in to skin-specific functionality inn the future?
    //////////////////////////////////////////////////////////////////////////

    // Tag corresponding to the GC that is responsible for how this projectile looks
    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    FGameplayTag SpawnGameplayCue;

    // Impact effects
    // Potentially could be slotted into a struct
    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    FGameplayTag HitWorldCue;

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    FGameplayTag HitCharacterCue;

	// Effect played on the character that was hit, looks like post processing
	UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
	FGameplayTag CameraImpactCue;

    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams")
    FGameplayTag HitNoneCue;

    bool IsValid() { return MaxRange > -1 || bIsHomingProjectile; }
};

UCLASS(config=Game)
class AAgoraProjectileBase : public AActor
{
	GENERATED_BODY()

public:

	AAgoraProjectileBase();

	/**
	 * When we're applying the gameplay effect
	 * Should be used sparingly to handle more complex scenarios/when 1 GE is not enough
	 */
	UPROPERTY(BlueprintAssignable, Category = "ProjectileEvents")
	FOnDeliverPayloadSignature OnDeliverPayload;
    
    // Actor
    virtual void Tick(float DeltaSeconds) override;
    // Override to avoid spawning a projectile twice on owning clients when we are trying to locally predict actions, we're telling the engine to skip replication for the owning actor
    bool IsNetRelevantFor(const AActor * RealViewer, const AActor * ViewTarget, const FVector & SrcLocation) const override;
    // ~Actor

    UFUNCTION()
    void NotifyProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
    /** Handles situations where the projectile reaches max range */
    UFUNCTION()
    void NotifyReachedMaxRanged();
    UFUNCTION()
    void NotifyTargetDied(const FDamageContext& DeathContext);
    UFUNCTION()
    void NotifyTargetChangedTeam(AActor* Actor);
    UFUNCTION()
    void NotifyOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /** Initializes the projecting with parameters */
    void Initialize(const FProjectileParams& Params);

    UFUNCTION(BlueprintPure, Category = "AgoraProjectile")
    AActor* GetHomingTarget();

    /** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

protected:

    // AActor
    virtual void BeginPlay() override;
    // ~AActor

    /** Sets up a homing target */
    void SetupHomingTarget(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "AgoraProjectile")
    bool IsValidPayloadTarget(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "AgoraProjectile")
    bool IsBlockedBy(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "AgoraProjectile")
    void DeliverPayloadTo(AActor* Actor, const FHitResult& HitResult);

    UFUNCTION(BlueprintPure, Category = "AgoraProjectile")
    bool IsHomingProjectie();
    
    UFUNCTION(BlueprintCallable, Category = "AgoraProjectile")
    void EndProjectile();

    /*
     * It is worth to note that since we are not replicating projectiles back down from the server (to allow for scuffed prediction)
     * updates to this structure will not propagate back down to the owning client
     */
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "AgoraProjectile", meta = (ExposeOnSpawn = "true"))
    FProjectileParams ProjectileParams;

    
    IDamageReceiverInterface* HomingTargetAsDamageReceiverInterface;

    /**
     * Where the projectile was fired from, used to track max ranges
     * This is only replicated because we want to pool this class
     * thus the actor will always exist on the client, and this will need to be updated
     */
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "AgoraProjectile")
    FVector StartLocation = FVector::ZeroVector;

    /** Sphere collision component */
    UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
    class USphereComponent* CollisionComp;

    /** Projectile movement component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    class UProjectileMovementComponent* ProjectileMovement;

    /**
     * The GameplayEffect to be applied on overlap
     * Not included in the data struct, as this is really only relevant on the server, the clients don't really care for it (unless they themselves are predicting)
     * (as far as I know)
     */
    UPROPERTY(BlueprintReadWrite, Category = "ProjectileParams", meta = (ExposeOnSpawn = "true"))
    FGameplayEffectSpecHandle GameplayEffectPayload;

    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraProjectile")
    void ReceiveEndProjectile();
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraProjectile")
    void ReceiveTargetDied(const FDamageContext& DeathContext);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraProjectile")
    void ReceiveTargetTeamChanged(AActor* ChangedActor);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraProjectile")
    void ReceiveProjectileOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraProjectile")
    void ReceiveReachedMaxRange();
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraProjectile")
    void ReceiveProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

};

