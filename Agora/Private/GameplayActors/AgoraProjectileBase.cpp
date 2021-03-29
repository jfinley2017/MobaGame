#include "AgoraProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "GameplayAbilities/Public/AbilitySystemBlueprintLibrary.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayAbilities/Public/GameplayCueManager.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
#include "GameplayAbilities/Public/AbilitySystemGlobals.h"

#include "Agora.h"
#include "AgoraTeamLibrary.h"
#include "DamageReceiverInterface.h"
#include "TeamInterface.h"
#include "Components/PrimitiveComponent.h"


DEFINE_LOG_CATEGORY(AgoraProjectile);

AAgoraProjectileBase::AAgoraProjectileBase() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
    CollisionComp->SetGenerateOverlapEvents(true);
    CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
    CollisionComp->SetCollisionProfileName("Projectile");
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AAgoraProjectileBase::NotifyOverlapped);		
    CollisionComp->OnComponentHit.AddDynamic(this, &AAgoraProjectileBase::NotifyProjectileHit);

    CollisionComp->CanCharacterStepUpOn = ECB_No;
	RootComponent = CollisionComp;
    SetRootComponent(CollisionComp);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->SetActive(false);

    // Defaults here are based on a basic attack
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 13000.f;
	ProjectileMovement->MaxSpeed = 13000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->Velocity = FVector(1.0f, 0.0f, 0.0f);
	
    // Ticking to allow for distance tracking
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(true);

    // Set replication
    SetReplicateMovement(true);
    SetReplicates(true);
}

void AAgoraProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAgoraProjectileBase, ProjectileParams);
    DOREPLIFETIME(AAgoraProjectileBase, StartLocation);
}

void AAgoraProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    // Blueprints shouldnt need to call initialize, it'll be done for them here.
    Initialize(ProjectileParams);
}

void AAgoraProjectileBase::SetupHomingTarget(AActor* Target)
{

    if (!Target || !IsValid(Target))
    {
        EndProjectile();
        return;
    }

    // We're only using tick to check if the target is alive every so often, no need to do this every frame.
    PrimaryActorTick.TickInterval = .25f;

    // Needed for homing to work
    CollisionComp->SetSimulatePhysics(false);
    ProjectileMovement->HomingTargetComponent = Target->GetRootComponent();
    ProjectileMovement->bIsHomingProjectile = true;
    // this will probably need to be specified by the caller eventually
    ProjectileMovement->HomingAccelerationMagnitude = 1000000.0f;

    HomingTargetAsDamageReceiverInterface = Cast<IDamageReceiverInterface>(GetHomingTarget());
    if (HomingTargetAsDamageReceiverInterface)
    {
        if (HomingTargetAsDamageReceiverInterface->IsDead())
        {
            EndProjectile();
            return;
        }
        HomingTargetAsDamageReceiverInterface->GetDeathDelegate().AddDynamic(this, &AAgoraProjectileBase::NotifyTargetDied);
    }

    ITeamInterface* TargetAsTeamInterface = Cast<ITeamInterface>(GetHomingTarget());
    if (TargetAsTeamInterface)
    {
        TargetAsTeamInterface->GetTeamChangedDelegate().AddDynamic(this, &AAgoraProjectileBase::NotifyTargetChangedTeam);
    }
}

void AAgoraProjectileBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (IsHomingProjectie())
    {
        if (!IsValid(GetHomingTarget()) || ( HomingTargetAsDamageReceiverInterface && HomingTargetAsDamageReceiverInterface->IsDead() ) )
        {
            EndProjectile();
        }
    }

    if (ProjectileParams.MaxRange > -1.0f 
        && FMath::Abs(FVector::Distance(StartLocation, GetActorLocation())) > ProjectileParams.MaxRange)
    {
        NotifyReachedMaxRanged();
    }
}

bool AAgoraProjectileBase::IsNetRelevantFor(const AActor * RealViewer, const AActor * ViewTarget, const FVector & SrcLocation) const
{
    Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
    return !IsOwnedBy(ViewTarget);
}

void AAgoraProjectileBase::NotifyTargetDied(const FDamageContext& DeathContext)
{
    ReceiveTargetDied(DeathContext);
    EndProjectile();
}

void AAgoraProjectileBase::NotifyTargetChangedTeam(AActor* Actor)
{
    ReceiveTargetTeamChanged(Actor);
}

void AAgoraProjectileBase::Initialize(const FProjectileParams& Params)
{
    ProjectileParams = Params;   
    ProjectileMovement->MaxSpeed = Params.MaxSpeed;
    ProjectileMovement->InitialSpeed = Params.InitialSpeed;
    CollisionComp->SetSphereRadius(Params.CollisionRadius);
    StartLocation = GetActorLocation();

    if (Params.bIsHomingProjectile)
    {
        if (!Params.HomingTarget && !Params.HomingTarget_Component.Get())
        {
            // Errored
            TRACE(AgoraProjectile, Warning, "Projectile was marked as homing, but had no target. Destroying.")
            EndProjectile();
            return;
        }
        AActor* HomingTargetActual = Params.HomingTarget ? Params.HomingTarget : Params.HomingTarget_Component->GetOwner();
        SetupHomingTarget(HomingTargetActual);
    }

    if (Params.bPassesThroughTerrain)
    {
        CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
    }
    
    if (UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(GetWorld()))
    {
        UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
            this,
            ProjectileParams.SpawnGameplayCue,
            EGameplayCueEvent::OnActive,
            FGameplayCueParameters());
    }
    

}

void AAgoraProjectileBase::NotifyOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    TRACE(AgoraProjectile, VeryVerbose, "%s projectile overlapped %s", *GetNameSafe(this), *GetNameSafe(OtherActor));

    if (IsValidPayloadTarget(OtherActor))
    {
        DeliverPayloadTo(OtherActor, SweepResult);
        EndProjectile();
    }
    else if (IsBlockedBy(OtherActor))
    {
        if (UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(GetWorld()))
        {
            UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
                this,
                ProjectileParams.SpawnGameplayCue,
                EGameplayCueEvent::Removed,
                FGameplayCueParameters());

            UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
                this,
                ProjectileParams.HitCharacterCue,
                EGameplayCueEvent::Executed,
                FGameplayCueParameters());
        }
        
        EndProjectile();
    }
    return;
    
}

void AAgoraProjectileBase::NotifyProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    TRACE(AgoraProjectile, VeryVerbose, "%s projectile hit %s", *GetNameSafe(this), *GetNameSafe(OtherActor));

    ReceiveProjectileHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

    if (UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(GetWorld()))
    {
        UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
            this,
            ProjectileParams.SpawnGameplayCue,
            EGameplayCueEvent::Removed,
            FGameplayCueParameters());

        UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
            this,
            ProjectileParams.HitWorldCue,
            EGameplayCueEvent::Executed,
            FGameplayCueParameters());
    }
    

    EndProjectile();
}

bool AAgoraProjectileBase::IsValidPayloadTarget(AActor* Actor)
{
    // Homing

    // Deal damage to the homing target no matter what
    if (IsHomingProjectie() && Actor == GetHomingTarget())
    {
        return true;
    }

    if (IsHomingProjectie() && ProjectileParams.bIgnoreAllButTarget)
    {
        return false;
    }

    // "Normal" projectile. Ala, we're a homing projectile that can hit things inbetween our
    // target and ourselves, or we're a standard fire-and-forget projectile

    bool IsSelf = Actor == GetInstigator();
    bool IsFriendlyToInstigator = UAgoraTeamLibrary::IsFriendly(Actor, GetInstigator());

    // We hit ourslves and we arent allowed to hit ourselves
    if (IsSelf && !ProjectileParams.bAllowSelfHit)
    {
        return false;
    }

    // We hit a teammate and we arent allowed to hit a teammate
    if (IsFriendlyToInstigator && !ProjectileParams.bAllowFriendlyFire)
    {
        return false;
    }

    return true;
}

bool AAgoraProjectileBase::IsBlockedBy(AActor* Actor)
{
    // Not delivering payload, but want to destroy anyways

    // We're a homing projectile and we only care about the homing target
    if (IsHomingProjectie() && ProjectileParams.bIgnoreAllButTarget && Actor != GetHomingTarget())
    {
        return false;
    }

    bool bIsFriendly = UAgoraTeamLibrary::IsFriendly(Actor, GetInstigator());

    if (GetOwner() == Actor)
    {
        return false;
    }

    // unfriendly targets always block
    if (!bIsFriendly)
    {
        return true;
    }

    // Target is a minion and we're shooting through minions
    if (UAgoraGameplayTags::IsMinion(Actor) && ProjectileParams.bPassesThroughFriendlyMinions)
    {
        return false;
    }

    // Target is a tower and we're shooting through towers
    if (UAgoraGameplayTags::IsTower(Actor) && ProjectileParams.bPassesThroughFriendlyTowers)
    {
        return false;
    }



    return true;

}

void AAgoraProjectileBase::DeliverPayloadTo(AActor* Actor, const FHitResult& SweepResult)
{
    if (HasAuthority())
    {
        UAbilitySystemComponent* OwningAbilityComp = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
        UAbilitySystemComponent* TargetAbilityComp = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
        FGameplayEffectSpec* EffectSpec = GameplayEffectPayload.Data.Get();

		OnDeliverPayload.Broadcast(Actor);

        if (OwningAbilityComp && EffectSpec)
        {
            FGameplayEffectSpec Spec = *EffectSpec;
            Spec.GetContext().AddHitResult(SweepResult);
            OwningAbilityComp->ApplyGameplayEffectSpecToTarget(Spec, TargetAbilityComp);
        }
    }

    if (UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(GetWorld()))
    {
        if (UAgoraGameplayTags::IsHero(Actor))
        {
            UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
                Actor,
                ProjectileParams.CameraImpactCue,
                EGameplayCueEvent::Executed,
                FGameplayCueParameters());
        }

        UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
            this,
            ProjectileParams.SpawnGameplayCue,
            EGameplayCueEvent::Removed,
            FGameplayCueParameters());


        UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
            this,
            ProjectileParams.HitCharacterCue,
            EGameplayCueEvent::Executed,
            FGameplayCueParameters());
    }
}

bool AAgoraProjectileBase::IsHomingProjectie()
{
    return ProjectileParams.bIsHomingProjectile;
}

AActor* AAgoraProjectileBase::GetHomingTarget()
{
    if (!ProjectileParams.HomingTarget_Component.IsValid() && !ProjectileParams.HomingTarget)
    {
        return nullptr;
    }
    return ProjectileParams.HomingTarget ? ProjectileParams.HomingTarget : ProjectileParams.HomingTarget_Component.Get()->GetOwner();
}

void AAgoraProjectileBase::NotifyReachedMaxRanged()
{
    ReceiveReachedMaxRange();
    if (UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(GetWorld()))
    {
        UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
            this,
            ProjectileParams.SpawnGameplayCue,
            EGameplayCueEvent::Removed,
            FGameplayCueParameters());

        UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
            this,
            ProjectileParams.HitNoneCue,
            EGameplayCueEvent::Executed,
            FGameplayCueParameters());
    }
    EndProjectile();
}


void AAgoraProjectileBase::EndProjectile()
{
    if (!ProjectileParams.IsValid() && !HasAuthority()) { return; }

    ReceiveEndProjectile();
    SetActorTickEnabled(false);
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    ProjectileMovement->SetActive(false);
    SetLifeSpan(.5f);
}

