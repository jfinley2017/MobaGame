// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraHeroBase.h"
#include "Agora.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/GameMode.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

#include "AgoraPlayerState.h"
#include "AgoraHeroMetadata.h"
#include "AgoraSkin.h"
#include "AgoraGameplayTags.h"
#include "AgoraXpLevelComponent.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraDataSingleton.h"
#include "GameFramework/PlayerController.h"
#include "AgoraMaterialEffectsComponent.h"
#include "GASCharacterMovementComponent.h"
#include "AgoraAbilitySystemComponent.h"
#include "Components/AgoraVisionComponent.h"
#include "AgoraTargetActor_Trace.h" // For FThirdPersonTrace, not entirely ideal
#include "AgoraInventoryComponent.h"

AAgoraHeroBase::AAgoraHeroBase(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->ProbeChannel = ECC_Visibility; // prevent from colliding with characters/etc
    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->bEnableCameraLag = true;
    SpringArmComponent->CameraLagSpeed = 5.0f;
    SpringArmComponent->CameraLagMaxDistance = 20.0f;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent);

    VisionComponent->SetSphereRadius(2000.0f);

    Inventory = CreateDefaultSubobject<UAgoraInventoryComponent>(TEXT("Inventory"));

}

void AAgoraHeroBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAgoraHeroBase, HeroSkin);
}

void AAgoraHeroBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	// Might make more sense to do this in player controller
    if (IsLocallyControlled())
    {
        AActor* NewFocusedActor = FindFocusActor();
        SetFocusedActor(NewFocusedActor);
    }
}

void AAgoraHeroBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    OnInputComponentReady.Broadcast(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AAgoraHeroBase::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AAgoraHeroBase::MoveRight);
    PlayerInputComponent->BindAxis("LookUp", this, &AAgoraHeroBase::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("Turn", this, &AAgoraHeroBase::AddControllerYawInput);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AAgoraHeroBase::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &AAgoraHeroBase::StopJumping);
    PlayerInputComponent->BindAction("ConfirmInput", IE_Pressed, GetAbilitySystemComponent(), &UAbilitySystemComponent::TargetConfirm);
    PlayerInputComponent->BindAction("CancelInput", IE_Pressed, GetAbilitySystemComponent(), &UAbilitySystemComponent::TargetCancel);

    // The delegate just allows us to bind a param to avoid making 4 individual functions
    // Maybe we could also use BindAbilityActivationToInputComponent again here with a different enum but I don't trust it
    PlayerInputComponent->BindAction<FUpgradeAbilityDelegate>("UpgradeAbilityAlternate", IE_Pressed, this, &AAgoraHeroBase::UpgradeAbility, EAbilityInput::AbilityAlternate);
    PlayerInputComponent->BindAction<FUpgradeAbilityDelegate>("UpgradeAbilityOne", IE_Pressed, this, &AAgoraHeroBase::UpgradeAbility, EAbilityInput::AbilityOne);
    PlayerInputComponent->BindAction<FUpgradeAbilityDelegate>("UpgradeAbilityTwo", IE_Pressed, this, &AAgoraHeroBase::UpgradeAbility, EAbilityInput::AbilityTwo);
    PlayerInputComponent->BindAction<FUpgradeAbilityDelegate>("UpgradeAbilityUltimate", IE_Pressed, this, &AAgoraHeroBase::UpgradeAbility, EAbilityInput::AbilityUltimate);

    GetAbilitySystemComponent()->BindAbilityActivationToInputComponent(PlayerInputComponent, FGameplayAbilityInputBinds("ConfirmInput", "CancelInput", "EAbilityInput"));
}

FVector AAgoraHeroBase::GetPawnViewLocation() const
{
    return CameraComponent && IsPlayerControlled() ? CameraComponent->GetComponentLocation() : Super::GetPawnViewLocation();
}

void AAgoraHeroBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// This is a hack to make sure that skins are set for the server in p2p/PIE games
	if (HasAuthority() && GetPlayerState())
	{
		AssignSkin();
	}
}

void AAgoraHeroBase::SetTeam(ETeam NewTeam)
{
    if (!HasAuthority()) { return; }

    Super::SetTeam(NewTeam);

    AAgoraPlayerState* PlayerStateAsAgoraPlayerState = GetPlayerState<AAgoraPlayerState>();
    if (PlayerStateAsAgoraPlayerState && PlayerStateAsAgoraPlayerState->GetTeam() != NewTeam) { PlayerStateAsAgoraPlayerState->SetTeam(NewTeam); }
}

void AAgoraHeroBase::NotifyTeamChanged(AActor* ChangedActor)
{
    SetFocusedActor(nullptr);
    ReceiveTeamChanged(GetTeam());
}

USpringArmComponent* AAgoraHeroBase::GetSpringArmComponent()
{
    return SpringArmComponent;
}

UCameraComponent* AAgoraHeroBase::GetCameraComponent()
{
    return CameraComponent;
}

void AAgoraHeroBase::SetSkin(UAgoraSkin* Skin)
{
	TRACE(AgoraCharacter, Log, "Setting skin %s on %s", *Skin->GetName(), *GetName());

	USkeletalMesh* SkinMesh = HeroSkin->SkinMesh.LoadSynchronous();
	GetMesh()->SetSkeletalMesh(SkinMesh);
	GetMesh()->SetAnimInstanceClass(GetHeroMetaData()->AnimClass);

	// This is required otherwise the damage flash effect will crash the game
	MaterialEffects->SetMeshMaterialsDynamic();
}

void AAgoraHeroBase::ServerAssignSkin_Implementation()
{
	AssignSkin();
}

bool AAgoraHeroBase::ServerAssignSkin_Validate()
{
	return true;
}

bool AAgoraHeroBase::GetRespawnTime(float& OutSpawnTimestamp)
{
    if (!IsDead())
    {
        OutSpawnTimestamp = -1.0f;
        return false;
    }

    FGameplayEffectQuery RespawnTimeQuery;
    FGameplayTagContainer DeathTags(UAgoraGameplayTags::Dead());
    FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(DeathTags);
    TArray< float > Durations = GetAbilitySystemComponent()->GetActiveEffectsTimeRemaining(Query);

    if (Durations.Num() != 0)
    {
        Durations.Sort();
        OutSpawnTimestamp = UAgoraBlueprintFunctionLibrary::GetTimeSinceStartOfGame(this) + Durations[Durations.Num() - 1];
        return true;
    }

    OutSpawnTimestamp = -1.0f;
    return false;

}

void AAgoraHeroBase::MoveForward(float RelativeSpeed)
{
    GetGASCMC()->ForwardAxisValue = RelativeSpeed;
    GetGASCMC()->PlayerMovementInput(true);
    //AddMovementInput(GetActorForwardVector() * RelativeSpeed);
}

void AAgoraHeroBase::MoveRight(float RelativeSpeed)
{
    GetGASCMC()->RightAxisValue = RelativeSpeed;
    GetGASCMC()->PlayerMovementInput(false);
    //AddMovementInput(GetActorRightVector() * RelativeSpeed);
}

AActor* AAgoraHeroBase::FindFocusActor()
{
	FHitResult Hit;
	// This was supposed to prevent us from focusing actors in between the camera and our actor, and it could probably be made to work
	FThirdPersonTrace Trace = FThirdPersonTrace(GetController(), this, GetActorLocation(), 1300.0f);
	
    FCollisionQueryParams TraceParams(FName(TEXT("FocusTrace")), true, this);
    TraceParams.bReturnPhysicalMaterial = false;
    TraceParams.bTraceComplex = true;
	
	FAgoraSweepQueryParams SweepParams;
	// The focusable interface checks if they are enemies
	SweepParams.TeamRule = ESweepTeamRule::Enemies;

	Trace.Trace(Hit, SweepParams, TraceParams, ECC_Damage);

	if (Hit.GetActor() && Cast<IFocusableInterface>(Hit.GetActor()))
	{
		return Hit.GetActor();
	}

	return nullptr;
}

bool AAgoraHeroBase::CanUpgradeAbility(EAbilityInput InputId)
{
    if (GetAgoraAbilitySystemComponent()->GetNumericAttribute(UAgoraAttributeSetEconomyBase::GetUnspentLevelPointsAttribute()) <= 0) { return false; }
    return GetAgoraAbilitySystemComponent()->CanUpgradeAbility(InputId);
}

void AAgoraHeroBase::UpgradeAbility(EAbilityInput AbilityIndex)
{
    if (!CanUpgradeAbility(AbilityIndex)) { return; }

    ServerUpgradeAbility(AbilityIndex);

}

void AAgoraHeroBase::ServerUpgradeAbility_Implementation(EAbilityInput AbilityIndex)
{
    if (!CanUpgradeAbility(AbilityIndex))
    {
        return;
    }

    GetAgoraAbilitySystemComponent()->UpgradeGameplayAbility(AbilityIndex);

    float CurrentUnspentLevelPoints = GetAgoraAbilitySystemComponent()->GetNumericAttribute(UAgoraAttributeSetEconomyBase::GetUnspentLevelPointsAttribute());
    GetAgoraAbilitySystemComponent()->SetNumericAttributeBase(UAgoraAttributeSetEconomyBase::GetUnspentLevelPointsAttribute(), CurrentUnspentLevelPoints - 1);
}

bool AAgoraHeroBase::ServerUpgradeAbility_Validate(EAbilityInput AbilityIndex)
{
    return true;
}

void AAgoraHeroBase::SetFocusedActor(AActor* NewFocus)
{
    // exit early if we're still focusing the same target
    if (NewFocus == CurrentlyFocusedActor) { return; }

    IFocusableInterface* NewFocusedActorAsFocusable = Cast<IFocusableInterface>(NewFocus);
    IFocusableInterface* CurrentlyFocusedActorAsFocusable = Cast<IFocusableInterface>(CurrentlyFocusedActor);

    if ((NewFocus && !NewFocusedActorAsFocusable) || (CurrentlyFocusedActor && !CurrentlyFocusedActorAsFocusable)) { ensure(false); }

    if (NewFocus != CurrentlyFocusedActor)
    {
        if (CurrentlyFocusedActor)
        {
            CurrentlyFocusedActorAsFocusable->Execute_NotifyEndFocused(CurrentlyFocusedActor, this);

        }

        if (NewFocus)
        {
            NewFocusedActorAsFocusable->Execute_NotifyFocused(NewFocus, this);
        }

        OnFocusChanged.Broadcast(NewFocus);
        CurrentlyFocusedActor = NewFocus;
    }
}

void AAgoraHeroBase::OnRep_HeroSkin()
{
	SetSkin(HeroSkin);
}

void AAgoraHeroBase::AssignSkin()
{
	// For now, we don't support changing skins mid-game. Should help prevent odd behaviors and make race conditions clearer
	if (HeroSkin)
	{
		return;
	}

    AAgoraPlayerState* OwningPlayerState = GetPlayerState<AAgoraPlayerState>();

    FPlayerData OwningPlayerData;
    if (OwningPlayerState)
    {
        OwningPlayerData = OwningPlayerState->GetPlayerData();

		// If we can't get from GI, try to get from PS
		if (!OwningPlayerData.HeroSkin)
		{
			OwningPlayerData = OwningPlayerState->GetPlayerData();
		}
    }
    
    UAgoraHeroMetadata* Metadata = GetHeroMetaData();

    if (HasAuthority())
    {
        if (OwningPlayerData.HeroSkin)
        {
			HeroSkin = OwningPlayerData.HeroSkin;
			OnRep_HeroSkin();
        }

        // Use default skin if testing in PIE
        if (Metadata && !HeroSkin)
        {
            if (Metadata->Skins.Num() > 0)
            {
                HeroSkin = Metadata->Skins[0];

				// Call the onrep, since this is generally from PIE where we are the server and the OnRep will not trigger
				OnRep_HeroSkin();
            }
            else
            {
                TRACE(Agora, Error, "AgoraCharacter requires HeroMetadata to be set for player controlled characters");
            }
        }
    }
}

UAgoraHeroMetadata* AAgoraHeroBase::GetHeroMetaData()
{
    return HeroMetadata;
}

float AAgoraHeroBase::CalculateDeathTimer()
{
    float BaseRespawnTime = 7.0f;
    float ScaleTimeByLevel = GetLevel() * 2;
    float ScaleDelayByTime = 1 + (UAgoraBlueprintFunctionLibrary::GetTimeSinceStartOfGame(this) / 60.0f * 0.03);
    return BaseRespawnTime + (ScaleTimeByLevel * ScaleDelayByTime);
}

void AAgoraHeroBase::SetupRespawn(const FDamageContext& DamageContext)
{
    float TimeDead = CalculateDeathTimer();
    TSubclassOf<UGameplayEffect> GameplayEffect = UAgoraBlueprintFunctionLibrary::GetGlobals()->DeathGameplayEffect;
    FGameplayEffectSpec DeathSpec(GameplayEffect->GetDefaultObject<UGameplayEffect>(), DamageContext.DamagingEffectContext);
    DeathSpec.SetSetByCallerMagnitude(UAgoraGameplayTags::SetByCallerDuration(), TimeDead);
    GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(DeathSpec);
}

void AAgoraHeroBase::FinishPlayDying()
{
    SetActorHiddenInGame(true);
}

void AAgoraHeroBase::FinishDying()
{
    AGameModeBase* GM = GetWorld()->GetAuthGameMode();
    if (GM)
    {
        AActor* Start = GM->ChoosePlayerStart(GetController());
		if (Start) {
			SetActorLocation(Start->GetActorLocation());
		}
    }
}

void AAgoraHeroBase::NotifyPlayDeathEffects(const FDamageContext& DamageContext)
{
    ReceivePlayDeathEffects(DamageContext);

    DisableInput(nullptr);
    PlayAnimMontage(DeathAnim);
    GetMesh()->GetAnimInstance()->Montage_GetBlendingOutDelegate()->BindLambda(
        [this](UAnimMontage* Montage, bool bWasInterrupted) mutable
        {
            if (Montage == DeathAnim) 
            { 
                this->FinishPlayDying(); 
            }
        }
    );

    FGameplayCueParameters DeathCueParams;
    DeathCueParams.EffectContext = DamageContext.DamagingEffectContext;
    UAgoraBlueprintFunctionLibrary::PlayLocalGameplayCueForActor(this, DeathGameplayCue, DeathCueParams);
}

void AAgoraHeroBase::NotifyPlayRespawnEffects()
{
    UAgoraBlueprintFunctionLibrary::PlayLocalGameplayCueForActor(this, RespawnGameplayCue, FGameplayCueParameters());
    PlayAnimMontage(RespawnAnim);

    ReceivePlayRespawnEffects();
}
