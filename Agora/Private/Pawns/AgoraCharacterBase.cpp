// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#include "AgoraCharacterBase.h"
#include "Agora.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PawnMovementComponent.h"
#include "AIModule/Classes/BrainComponent.h"
#include "AIModule/Classes/AIController.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemBlueprintLibrary.h" 

#include "Modes/AgoraGameInstance.h"
#include "AgoraAbilitySystemComponent.h"
#include "AgoraVisionLibrary.h"
#include "AgoraBlueprintFunctionLibrary.h" 
#include "AgoraGameplayTags.h"
#include "AgoraGameMode.h"
#include "Components/AgoraSceneTargetComponent.h"
#include "Components/AgoraXpLevelComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AgoraMaterialEffectsComponent.h"
#include "Components/GASCharacterMovementComponent.h"
#include "Components/AgoraVisionComponent.h"
#include "Components/AgoraTeamComponent.h"
#include "Components/AgoraDeathRewardComponent.h"
#include "AgoraDataSingleton.h"
#include "Controllers/AgoraPlayerController.h" 
#include "AgoraMapIconComponent.h"



AAgoraCharacterBase::AAgoraCharacterBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UGASCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    SetReplicates(true);
    bAlwaysRelevant = true;
	GetGASCMC()->SetIsReplicated(true);

	// The ALS handles turning in place. We don't need this to be enabled.
    bUseControllerRotationYaw = false;

    // Damage traces (such as the auto attack line trace) use visibility. This allows us to get a result when we are aiming at characters
	GetCapsuleComponent()->SetCollisionProfileName("Pawn");

    // We're using the capsule component as our collision, not the mesh
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // #TODO - Currently vision status and revealing is combined into one component - agoravisioncomponent. separate them.
    VisionComponent = CreateDefaultSubobject<UAgoraVisionComponent>("VisionComponent");
    VisionComponent->SetupAttachment(RootComponent);

    MapIconComponent = CreateDefaultSubobject<UAgoraMapIconComponent>("MapIconComponent");
    TeamComponent = CreateDefaultSubobject<UAgoraTeamComponent>("TeamComponent");
    LevelComponent = CreateDefaultSubobject<UAgoraXpLevelComponent>("LevelComponent");
    AbilitySystemComponent = CreateDefaultSubobject<UAgoraAbilitySystemComponent>("AbilitySystemComponent");
    TargetLocationComponent = CreateDefaultSubobject<UAgoraSceneTargetComponent>(TEXT("TargetLocationComponent"));
	TargetLocationComponent->SetupAttachment(RootComponent);
	MaterialEffects = CreateDefaultSubobject<UAgoraMaterialEffectsComponent>(TEXT("MaterialEffects"));
	DeathRewardComponent = CreateDefaultSubobject<UAgoraDeathRewardComponent>("DeathRewardComponent");
}

void AAgoraCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

UAgoraMapIconComponent* AAgoraCharacterBase::GetMapIconComponent()
{
    return MapIconComponent;
}

void AAgoraCharacterBase::OnConstruction(const FTransform& Transform)
{
	GetGASCMC()->LastVelocityRotation = GetActorRotation();
	GetGASCMC()->LookingRotation = GetActorRotation();
	GetGASCMC()->LastMovementInputRotation = GetActorRotation();
	GetGASCMC()->TargetRotation = GetActorRotation();
	GetGASCMC()->CharacterRotation = GetActorRotation();
}


void AAgoraCharacterBase::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);  // Important. Can't activate abilities without this
    SetUpAttributeSets();
}

void AAgoraCharacterBase::SetUpAttributeSets()
{
    if (BaseAttributeSetClass)
    {
        UAgoraAttributeSetBase* BaseSet = NewObject<UAgoraAttributeSetBase>(GetAbilitySystemComponent()->OwnerActor, BaseAttributeSetClass);
        GetAbilitySystemComponent()->AddDefaultSubobjectSet(BaseSet);
    }
    if (HealthAttributeSetClass)
    {
        UAgoraAttributeSetHealth* HealthSet = NewObject<UAgoraAttributeSetHealth>(GetAbilitySystemComponent()->OwnerActor, HealthAttributeSetClass);
        GetAbilitySystemComponent()->AddDefaultSubobjectSet(HealthSet);
    }
    if (MovementAttributeSetClass)
    {
        UAgoraAttributeSetMovement* MovementSet = NewObject<UAgoraAttributeSetMovement>(GetAbilitySystemComponent()->OwnerActor, MovementAttributeSetClass);
        GetAbilitySystemComponent()->AddDefaultSubobjectSet(MovementSet);
    }
    if (MovementAttributeSetClass)
    {
        UAgoraAttributeSetStatsBase* StatsSet = NewObject<UAgoraAttributeSetStatsBase>(GetAbilitySystemComponent()->OwnerActor, StatsAttributeSetClass);
        GetAbilitySystemComponent()->AddDefaultSubobjectSet(StatsSet);
    }
    if (EconomyAttributeSetClass)
    {
        UAgoraAttributeSetEconomyBase* EconomySet = NewObject<UAgoraAttributeSetEconomyBase>(GetAbilitySystemComponent()->OwnerActor, EconomyAttributeSetClass);
        GetAbilitySystemComponent()->AddDefaultSubobjectSet(EconomySet);
    }
    for (TSubclassOf<UAgoraAttributeSetBase> NewSetClass : AdditionalAttributeSetClasses)
    {
        UAgoraAttributeSetBase* NewSet = NewObject<UAgoraAttributeSetBase>(GetAbilitySystemComponent()->OwnerActor, NewSetClass);
        GetAbilitySystemComponent()->AddDefaultSubobjectSet(NewSet);
    }
}

void AAgoraCharacterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // GAS Boilerplate function
    // Refresh actor information on the ASC
    if (GetAbilitySystemComponent())
    {
        GetAbilitySystemComponent()->RefreshAbilityActorInfo();
	}
}

// Called when the game starts or when spawned
void AAgoraCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // Refreshing here because we need to be sure playercontroller/state/etc exists, and it may have not existed previously
    GetAbilitySystemComponent()->RefreshAbilityActorInfo();
    GetAbilitySystemComponent()->AddLooseGameplayTags(DefaultGameplayTags);
    GetAbilitySystemComponent()->RegisterGameplayTagEvent(UAgoraGameplayTags::Dead(), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAgoraCharacterBase::NotifyRespawned);

    LevelComponent->OnLeveledUp.AddDynamic(this, &AAgoraCharacterBase::NotifyLevelUp);
    TeamComponent->OnTeamChanged.AddDynamic(this, &AAgoraCharacterBase::NotifyTeamChanged);

    InstigatedDamageCue = FGameplayTag::RequestGameplayTag("GameplayCue.Events.Damage.Instigated");

    if (HasAuthority())
    {
        AbilitySystemComponent->AddStartupGameplayAbilities();
    }

}

void AAgoraCharacterBase::NotifyLevelUp(AActor* LeveledActor, int32 NewLevel, float LevelupTimestamp)
{
    ReceiveLevelUp(LeveledActor, NewLevel, LevelupTimestamp);

    UAgoraAbilitySystemComponent* AgoraAbilitySystemComponent = GetAgoraAbilitySystemComponent();

    if (HasAuthority())
    {
        float CurrentUnspentLevelPoints = GetAbilitySystemComponent()->GetNumericAttribute(UAgoraAttributeSetEconomyBase::GetUnspentLevelPointsAttribute());
        GetAbilitySystemComponent()->SetNumericAttributeBase(UAgoraAttributeSetEconomyBase::GetUnspentLevelPointsAttribute(), CurrentUnspentLevelPoints + 1);

        // Auto upgrade primary on level up
        AgoraAbilitySystemComponent->UpgradeGameplayAbility(EAbilityInput::AbilityPrimary);

        AgoraAbilitySystemComponent->SetBaseStatsFromCurveTable(LevelComponent->GetCurrentLevel(),
            AgoraAbilitySystemComponent->LevelStatsCurveTable);
    }

    if (UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(this))
    {
        FGameplayCueParameters LevelCueParams;
        LevelCueParams.RawMagnitude = GetLevel();
        LevelCueParams.Instigator = this;
        UAgoraBlueprintFunctionLibrary::PlayLocalGameplayCueForActor(this, LevelupGameplayCue, LevelCueParams);
    }
}


float AAgoraCharacterBase::CalculateDeathTimer()
{
    return 0.0f;
}   

void AAgoraCharacterBase::NotifyFocused_Implementation(AActor* FocusingActor)
{
    ReceiveFocused(FocusingActor);

    if (!UAgoraBlueprintFunctionLibrary::IsFriendly(FocusingActor, this))
    {
        GetMesh()->SetRenderCustomDepth(true);
        GetMesh()->SetCustomDepthStencilValue(254);
    }

    OnStartFocused.Broadcast(FocusingActor);

}

void AAgoraCharacterBase::NotifyEndFocused_Implementation(AActor* EndFocusingActor)
{
    ReceiveEndFocused(EndFocusingActor);

    GetMesh()->SetRenderCustomDepth(false);
    GetMesh()->SetCustomDepthStencilValue(0);

    OnEndFocused.Broadcast(EndFocusingActor);
}

FOnFocusedActorSignature& AAgoraCharacterBase::GetStartFocusedDelegate()
{
    return OnStartFocused;
}

FOnFocusedActorSignature& AAgoraCharacterBase::GetEndFocusedDelegate()
{
    return OnEndFocused;
}

FOnFocusedActorSignature& AAgoraCharacterBase::GetFocusChangedDelegate()
{
    return OnFocusChanged;
}

void AAgoraCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

float AAgoraCharacterBase::GetMaxSpeed()
{
    return GetAbilitySystemComponent()->GetSetChecked<UAgoraAttributeSetMovement>()->GetMaxSpeed();
}

UGASCharacterMovementComponent* AAgoraCharacterBase::GetGASCMC()
{
	return Cast<UGASCharacterMovementComponent>(GetCharacterMovement());
}



void AAgoraCharacterBase::NotifyRevealed(AActor* Revealer)
{
    VisionComponent->NotifyRevealed(Revealer);
}

void AAgoraCharacterBase::NotifyRevealedEnd(AActor* StoppedRevealer)
{
    VisionComponent->NotifyEndRevealed(StoppedRevealer);
}

bool AAgoraCharacterBase::IsVisibleFor(AActor* ViewingActor) const
{
    return UAgoraVisionLibrary::HasVisionOf(GetWorld(),ViewingActor, this);
}

void AAgoraCharacterBase::GetViewLocation(FVector& OutViewLocation) const
{
    FRotator ThrowAway;
    GetActorEyesViewPoint(OutViewLocation, ThrowAway);
}

uint8 AAgoraCharacterBase::GetVisionLevel() const
{
    return VisionComponent->GetVisionLevel();
}

uint8 AAgoraCharacterBase::GetHiddenLevel() const
{
    return VisionComponent->GetHiddenLevel();
}

FOnVisibilityToggledSignature& AAgoraCharacterBase::GetBecameVisibleDelegate()
{
    return VisionComponent->OnRevealed;
}

FOnVisibilityToggledSignature& AAgoraCharacterBase::GetBecameHiddenDelegate()
{
    return VisionComponent->OnHidden;
}

FOnVisionLevelChangedSignature& AAgoraCharacterBase::GetVisionLevelChangedDelegate()
{
    return VisionComponent->OnVisionLevelChanged;
}

FOnHiddenLevelChangedSignature& AAgoraCharacterBase::GetHiddenLevelChangedDelegate()
{
    return VisionComponent->OnHiddenLevelChanged;
}

float AAgoraCharacterBase::CalculateBasicAttackTime()
{
    const UAgoraAttributeSetStatsBase* StatsSet = GetAbilitySystemComponent()->GetSet<UAgoraAttributeSetStatsBase>();
    if (StatsSet)
    {
        return StatsSet->CalculateBasicAttackCooldown();
    }
    return 0.0f;
}

UAgoraSceneTargetComponent* AAgoraCharacterBase::GetTargetComponent()
{
    return TargetLocationComponent;
}

void AAgoraCharacterBase::StopPostDeath()
{
    GetMovementComponent()->StopMovementImmediately();
    GetAbilitySystemComponent()->CancelAllAbilities();
    GetMesh()->GetAnimInstance()->StopAllMontages(0.0f);
    GetMesh()->SetEnableGravity(false);
    GetCapsuleComponent()->SetEnableGravity(false);
    GetCharacterMovement()->Velocity = FVector::ZeroVector;
    GetCharacterMovement()->GravityScale = 0;
    DisableAllInput();
    DisableCollision();
}

void AAgoraCharacterBase::DisableAllInput()
{
    UAgoraBlueprintFunctionLibrary::CompletelyDisableInput(this);
}

void AAgoraCharacterBase::DisableCollision()
{
    
    GetCapsuleComponent()->SetEnableGravity(false);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAgoraCharacterBase::FinishPlayDying()
{
	ReceiveFinishPlayDying();
    return;
}

void AAgoraCharacterBase::FinishDying()
{
    return;
}

void AAgoraCharacterBase::ValidateCharacter()
{
    UAgoraBlueprintFunctionLibrary::EnableInput(this);
    GetMesh()->SetEnableGravity(true);
    GetMovementComponent()->Activate();
    GetCapsuleComponent()->SetEnableGravity(true);
    GetCharacterMovement()->GravityScale = 1;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetActorHiddenInGame(false);
}

void AAgoraCharacterBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    GetAbilitySystemComponent()->GetOwnedGameplayTags(TagContainer);
}

bool AAgoraCharacterBase::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
    return GetAbilitySystemComponent()->HasMatchingGameplayTag(TagToCheck);
}

bool AAgoraCharacterBase::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
    return GetAbilitySystemComponent()->HasAllMatchingGameplayTags(TagContainer);
}

bool AAgoraCharacterBase::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
    return GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(TagContainer);
}

uint8 AAgoraCharacterBase::GetLevel()
{
	return LevelComponent->GetCurrentLevel();
}

UAgoraLevelComponent* AAgoraCharacterBase::GetLevelComponent()
{
	return LevelComponent;
}

bool AAgoraCharacterBase::IsDead() const
{
    return HasMatchingGameplayTag(UAgoraGameplayTags::Dead());
}


FDiedSignature& AAgoraCharacterBase::GetDeathDelegate()
{
    return OnDeath;
}

UAbilitySystemComponent* AAgoraCharacterBase::GetAbilitySystemComponent() const
{
    UAbilitySystemComponent* RetComponent = AbilitySystemComponent;
    if (!RetComponent)
    {
        TRACE(AgoraCharacter, Fatal, "Invalid AbilitySystemComponent for %s", *GetNameSafe(this))
        return nullptr;
    }

    return AbilitySystemComponent;
}

UAgoraAbilitySystemComponent* AAgoraCharacterBase::GetAgoraAbilitySystemComponent() const
{
	return Cast<UAgoraAbilitySystemComponent>(GetAbilitySystemComponent());
}

void AAgoraCharacterBase::NotifyDamageReceived(float CurrentHP, float MaxHP, const FDamageContext& DamageContext)
{
    if (IsDead()) { return; }

    if (UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(this) && CurrentHP != MaxHP)
    {
        NotifyPlayDamagedEffects(CurrentHP, MaxHP, DamageContext);
    }

    // Blueprint functionality here
    OnDamageReceived.Broadcast(CurrentHP, MaxHP, DamageContext);
    ReceiveDamageReceived(CurrentHP, MaxHP, DamageContext);
}

void AAgoraCharacterBase::NotifyPlayDamagedEffects(float CurrentHP, float MaxHP, const FDamageContext& DamageContext)
{
    if ((Cast<APawn>(DamageContext.DamageInstigator) && Cast<APawn>(DamageContext.DamageInstigator)->IsLocallyControlled()) || IsLocallyControlled())
    {
        MaterialEffects->PlayHitFlash();
    }

    // HitReact
    if (DamageContext.DamagingEffectContext.GetHitResult() && HitReactAnim && CurrentHP > 0.0f)
    {
        FVector ActorRightVector = GetMesh()->GetRightVector();
        FVector HitResultNormal = DamageContext.DamagingEffectContext.GetHitResult()->Normal;
        FName HitDirection = UAgoraBlueprintFunctionLibrary::DetermineHitDirection(ActorRightVector, HitResultNormal);
        PlayAnimMontage(HitReactAnim, 1.f, HitDirection);
    }
    // HitReactSound
    UGameplayStatics::PlaySoundAtLocation(this, HitReactSound, GetActorLocation(), .7f);
}

void AAgoraCharacterBase::NotifyDamageInstigated(const FDamageContext& DamageContext)
{
    FGameplayCueParameters DamageInstigatedParams;
    DamageInstigatedParams.EffectContext = DamageContext.DamagingEffectContext;
    DamageInstigatedParams.RawMagnitude = DamageContext.Magnitude;
    UAgoraBlueprintFunctionLibrary::PlayLocalGameplayCueForActor(DamageContext.DamagedActor, InstigatedDamageCue, DamageInstigatedParams);

    OnDamageInstigated.Broadcast(DamageContext);
    ReceiveDamageInstigated(DamageContext);
}

FDamageReceivedSignature& AAgoraCharacterBase::GetDamageReceivedDelegate()
{
    return OnDamageReceived;
}

FDamageInstigatedSignature& AAgoraCharacterBase::GetDamageInstigatedDelegate()
{
    return OnDamageInstigated;
}

void AAgoraCharacterBase::NotifyPlayRespawnEffects()
{
    ReceivePlayRespawnEffects();

    UAgoraBlueprintFunctionLibrary::PlayLocalGameplayCueForActor(this, RespawnGameplayCue, FGameplayCueParameters());
    PlayAnimMontage(RespawnAnim);
}

void AAgoraCharacterBase::NotifyDied(const FDamageContext& DamageContext)
{
    if (!HasAuthority() || IsDead()) { return; }

    SetupRespawn(DamageContext);
	DeathRewardComponent->NotifyDied(DamageContext);
	NetMulticast_Death(DamageContext);

    FTimerHandle FinishDyingTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(FinishDyingTimerHandle, this, &AAgoraCharacterBase::FinishDying, 3.0f, false);
}

void AAgoraCharacterBase::SetupRespawn(const FDamageContext& DamageContext)
{
    return;
}

void AAgoraCharacterBase::NetMulticast_Death_Implementation(FDamageContext DamageContext)
{
    ReceiveDied(DamageContext);

    StopPostDeath();
    
    if (UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(this))
    {
        NotifyPlayDeathEffects(DamageContext);
    }

    OnDeath.Broadcast(DamageContext);
}

void AAgoraCharacterBase::NotifyPlayDeathEffects(const FDamageContext& DamageContext)
{
    // @todo fix this, currently the effect is not playing correctly
	//MaterialEffects->PlayDeathDisintegrate();

    ReceivePlayDeathEffects(DamageContext);

    if (DeathAnim && UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(this))
    {
        PlayAnimMontage(DeathAnim);
		FOnMontageBlendingOutStarted* BlendOutDelegate = GetMesh()->GetAnimInstance()->Montage_GetBlendingOutDelegate();
		
		// This can be null if you do something dumb like Slomo 10 and the death montage finishes before this delegate even gets a chance
		if (BlendOutDelegate)
		{
			BlendOutDelegate->BindLambda(
				[this](UAnimMontage* Montage, bool bWasInterrupted) mutable
			{
				if (Montage == DeathAnim) { this->FinishPlayDying(); }
			}
			);
		}

        FGameplayCueParameters DeathCueParams;
        DeathCueParams.EffectContext = DamageContext.DamagingEffectContext;
        UAgoraBlueprintFunctionLibrary::PlayLocalGameplayCueForActor(this, DeathGameplayCue, DeathCueParams);
    }
   
}

bool AAgoraCharacterBase::GetRespawnTime(float& OutSpawnTimestamp)
{
    return false;
}

void AAgoraCharacterBase::NotifyRespawned(FGameplayTag Tag, int32 Count)
{
    // Server only function
    if (!HasAuthority()) { return; }

    if (Count > 0)
    {
        // Any tag relating to death should never be > 1. There was an error somewhere.
        if (Count > 1)
        {
            TRACE(AgoraCharacter, Fatal, "%s has %s tag count of %d. Tag count should never be more than 1", *GetNameSafe(this), Count, *Tag.GetTagName().ToString());
        }
        return;
    }

    NetMulticast_CharacterRespawned();
}

void AAgoraCharacterBase::NetMulticast_CharacterRespawned_Implementation()
{
    ReceiveRespawned();

    ValidateCharacter();

    if (UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(this))
    {
        NotifyPlayRespawnEffects();
    }

    OnRespawned.Broadcast();
}

void AAgoraCharacterBase::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomNode /*= 0*/)
{
	GetGASCMC()->OnPawnMovementModeChanged(PrevMovementMode, GetGASCMC()->MovementMode, PreviousCustomNode, GetGASCMC()->CustomMovementMode);
}

ETeam AAgoraCharacterBase::GetTeam() const
{
    return TeamComponent->GetTeam();
}

void AAgoraCharacterBase::SetTeam(ETeam NewTeam)
{
    if (!HasAuthority()) { return; }

    TeamComponent->SetTeam(NewTeam);
    
}

void AAgoraCharacterBase::NotifyTeamChanged(AActor* ChangedActor)
{
    ReceiveTeamChanged(GetTeam());
}

FOnTeamChangedSignature& AAgoraCharacterBase::GetTeamChangedDelegate()
{
    return TeamComponent->OnTeamChanged;
}


