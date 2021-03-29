// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraVisionComponent.h"
#include "AbilitySystemInterface.h"
#include "DrawDebugHelpers.h"
#include "NoExportTypes.h"
#include "GameFramework/PlayerController.h"

#include "AgoraVisionLibrary.h"
#include "TeamInterface.h"
#include "AgoraGameplayTags.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraGameState.h"

DECLARE_CYCLE_STAT(TEXT("UpdateVisionOverlaps"), STAT_UpdateVisionOverlaps, STATGROUP_AgoraVision);

UAgoraVisionComponent::UAgoraVisionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickInterval = .25;
}

void UAgoraVisionComponent::BeginPlay()
{
    Super::BeginPlay();

    AAgoraGameState* AgoraGameState = GetWorld()->GetGameState<AAgoraGameState>();
    if (AgoraGameState)
    {
        if (!AgoraGameState->GetVisionService())
        {
            AgoraGameState->OnVisionServiceReady.AddDynamic(this, &UAgoraVisionComponent::NotifyVisionServiceReady);
        }

        NotifyVisionServiceReady(AgoraGameState->GetVisionService());
    }

  
    SetupOwner();
    SetupCollision();
}

void UAgoraVisionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateOverlappingVision(CurrentTeam, CurrentTeam);
}

void UAgoraVisionComponent::SetupCollision()
{
    if (!GetOwner()->HasAuthority())
    {
        SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SetGenerateOverlapEvents(false);
        PrimaryComponentTick.bCanEverTick = false;
        PrimaryComponentTick.bStartWithTickEnabled = false;
        return;
    }

	SetCollisionProfileName("OverlapOnlyPawn");

    OnComponentBeginOverlap.AddDynamic(this, &UAgoraVisionComponent::NotifyOverlapped);
    OnComponentEndOverlap.AddDynamic(this, &UAgoraVisionComponent::NotifyEndOverlapped);

    UpdateOverlappingVision(CurrentTeam, CurrentTeam);

}

void UAgoraVisionComponent::SetupOwner()
{
    OwnerAsTeamInterface = Cast<ITeamInterface>(GetOwner());
    if (OwnerAsTeamInterface)
    {
        OwnerAsTeamInterface->GetTeamChangedDelegate().AddDynamic(this, &UAgoraVisionComponent::NotifyOwnerTeamChanged);
        CurrentTeam = OwnerAsTeamInterface->GetTeam();
    }

    /* IAbilitySystemInterface* OwnerAsAbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
    if (OwnerAsAbilitySystemInterface)
    {
        CachedOwnerAbilitySystemComponent = OwnerAsAbilitySystemInterface->GetAbilitySystemComponent();
        OwnerAsAbilitySystemInterface->GetAbilitySystemComponent()->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag("Vision")).AddDynamic(this, &UAgoraVisionComponent::NotifyOwnerTagsChanged);
    }*/
}

void UAgoraVisionComponent::NotifyOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    IVisionInterface* OtherActorAsVisionInterface = Cast<IVisionInterface>(OtherActor);

    if (OtherActorAsVisionInterface && ShouldApplyVisionTo(OtherActor, false))
    {
        NotifyNewSpottedActor(OtherActor);
    }
}

void UAgoraVisionComponent::NotifyEndOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    IVisionInterface* OtherActorAsVisionInterface = Cast<IVisionInterface>(OtherActor);

    if (OtherActorAsVisionInterface && CurrentlySpottedActors.Contains(OtherActor))
    {
        NotifyRemoveSpottedActor(OtherActor);
    }
}

void UAgoraVisionComponent::NotifyNewSpottedActor(AActor* SpottedActor)
{
    CurrentlySpottedActors.AddUnique(SpottedActor);
    if (UAgoraVisionLibrary::GetVisionService(GetWorld()))
    {
        UAgoraVisionLibrary::GetVisionService(GetWorld())->NotifyGainedVision(GetOwner(), CurrentTeam, SpottedActor);
    }
}

void UAgoraVisionComponent::NotifyRemoveSpottedActor(AActor* RemovedSpottedActor)
{
    CurrentlySpottedActors.Remove(RemovedSpottedActor);
    if (UAgoraVisionLibrary::GetVisionService(GetWorld()))
    {
        UAgoraVisionLibrary::GetVisionService(GetWorld())->NotifyLostVision(GetOwner(), CurrentTeam, RemovedSpottedActor);
    }
}

void UAgoraVisionComponent::NotifyRevealed(AActor* RevealingActor)
{
    if (!bIsOwnerVisible)
    {
        bHasBeenPassedOnce = true;
        bIsOwnerVisible = true;
        OnRevealed.Broadcast();
    }
}

void UAgoraVisionComponent::NotifyEndRevealed(AActor* EndRevealingActor)
{
    if (bIsOwnerVisible)
    {
        bHasBeenPassedOnce = true;
        bIsOwnerVisible = false;
        OnHidden.Broadcast();
    }
}

void UAgoraVisionComponent::NotifyVisionServiceReady(AAgoraVisionService* VisionService)
{
    if (UAgoraVisionLibrary::LocalPlayerHasVisionOf(GetWorld(), GetOwner()))
    {
        NotifyRevealed(nullptr);
    }
    else
    {
        NotifyEndRevealed(nullptr);
    }
}

void UAgoraVisionComponent::NotifyOwnerTeamChanged(AActor* ChangedActor)
{
    PreviousTeam = CurrentTeam;
    CurrentTeam = OwnerAsTeamInterface->GetTeam();
    UpdateOverlappingVision(CurrentTeam, PreviousTeam);

    if (UAgoraVisionLibrary::LocalPlayerHasVisionOf(GetWorld(),GetOwner()))
    {
        NotifyRevealed(nullptr);
    }
    else
    {
        NotifyEndRevealed(nullptr);
    }
}

bool UAgoraVisionComponent::ShouldApplyVisionTo(AActor* Actor, bool bSkipLosCheck)
{
    IVisionInterface* OtherActorAsVisionInterface = Cast<IVisionInterface>(Actor);
    IVisionInterface* OwnerAsVisionInterface = Cast<IVisionInterface>(GetOwner());

    if (!OtherActorAsVisionInterface)
    {
        return false;
    }

    // Friendly units always have vision, will be handled by IsOwnerVisibleFor
    if (Actor == GetOwner() || UAgoraBlueprintFunctionLibrary::IsFriendly(GetOwner(), Actor))
    {
        return false;
    }

    if (OwnerAsVisionInterface->GetVisionLevel() < OtherActorAsVisionInterface->GetHiddenLevel())
    {
        return false;
    }

    if (!bSkipLosCheck)
    {
        bool bHasLineOfSightOf = OwnerHasLineOfSightTo(Actor);
        if (!bHasLineOfSightOf)
        {
            return false;
        }
    }

    return true;
}

uint8 UAgoraVisionComponent::GetVisionLevel() const
{
    return CurrentVisionLevel;
}

uint8 UAgoraVisionComponent::GetHiddenLevel() const
{
    return CurrentVisionLevel;
}

void UAgoraVisionComponent::UpdateOverlappingVision(ETeam TeamToGrantVisionTo, ETeam TeamToRemoveVisionFrom)
{
    SCOPE_CYCLE_COUNTER(STAT_UpdateVisionOverlaps);

    TArray<AActor*> OverlappedActors;
    GetOverlappingActors(OverlappedActors);

    for (AActor* Actor : OverlappedActors)
    {
        IVisionInterface* OverlappedAsVisionInterface = Cast<IVisionInterface>(Actor);
        if (!OverlappedAsVisionInterface) { continue; }

        bool bShouldApplyVisionToActor = ShouldApplyVisionTo(Actor, false);
        bool bCurrentlyTrackingActor = CurrentlySpottedActors.Contains(Actor);

        if (bShouldApplyVisionToActor && !bCurrentlyTrackingActor)
        {
            if (OverlappedAsVisionInterface)
            {
                TRACE(Agora, Log, "Added %s to vision", *GetNameSafe(Actor));
                CurrentlySpottedActors.AddUnique(Actor);
                if (UAgoraVisionLibrary::GetVisionService(GetWorld()))
                {
                    UAgoraVisionLibrary::GetVisionService(GetWorld())->NotifyGainedVision(GetOwner(), TeamToGrantVisionTo, Actor);
                }
                
            }
        }

        if (!bShouldApplyVisionToActor && bCurrentlyTrackingActor)
        {
            CurrentlySpottedActors.Remove(Actor);
            if (UAgoraVisionLibrary::GetVisionService(GetWorld()))
            {
                UAgoraVisionLibrary::GetVisionService(GetWorld())->NotifyLostVision(GetOwner(), TeamToRemoveVisionFrom, Actor);
            }
        }
    }
}

bool UAgoraVisionComponent::OwnerHasLineOfSightTo(AActor* Actor)
{
    IVisionInterface* OwnerAsVisionInterface = Cast<IVisionInterface>(GetOwner());
    if (!OwnerAsVisionInterface)
    {
        return false;
    }

    FVector OwnerViewLocation;
    OwnerAsVisionInterface->GetViewLocation(OwnerViewLocation);

    FCollisionQueryParams CollisionParms(SCENE_QUERY_STAT(LineOfSight), true, Actor);
    CollisionParms.AddIgnoredActor(GetOwner());
    FVector TargetLocation = Actor->GetActorLocation();
    bool bHit = GetWorld()->LineTraceTestByChannel(OwnerViewLocation, TargetLocation, ECC_Visibility, CollisionParms);
    if (!bHit)
    {
        return true;
    }
    return false;
}