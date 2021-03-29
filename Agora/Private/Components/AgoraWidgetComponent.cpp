// Fill out your copyright notice in the Description page of Project Settings.

#include "AgoraWidgetComponent.h"
#include "AgoraUserWidget.h"
#include "DamageReceiverInterface.h"
#include "AgoraCharacterBase.h"
#include "AgoraTypes.h"
#include "VisionInterface.h"
#include "TeamInterface.h"
#include "AgoraVisionLibrary.h"
#include "AgoraBlueprintFunctionLibrary.h"

void UAgoraWidgetComponent::BeginPlay()
{
    Super::BeginPlay();
   
    if (GetNetMode() == NM_DedicatedServer)
    {
        PrimaryComponentTick.bCanEverTick = false;
        return;
    }

    PrimaryComponentTick.bCanEverTick = true;
}

void UAgoraWidgetComponent::InitWidget()
{
    Super::InitWidget();

    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    SetupOwner();
}

void UAgoraWidgetComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    if (IsValid(GetOwner()) && !bInitialized)
    {
        SetupOwner();
        bInitialized = true;
        //PrimaryComponentTick.bCanEverTick = false;
    }

    //////////////////////////////////////////////////////////////////////////
    // Distance to local player checking 

    if (DrawBehavior != EDrawBehavior::VisibleAndLocalPlayerNear)
    {
        return;
    }

    float DistanceToLocalPlayer = 0.0f;
    if (DetermineDistanceToLocalPlayer(DistanceToLocalPlayer) && DistanceToLocalPlayer <= DrawDistance)
    {   
        if (!bLocalPlayerIsOverlapped)
        {
            NotifyLocalPlayerEnteredDrawDistance();
        }
    }
    else if(bLocalPlayerIsOverlapped)
    {
        NotifyLocalPlayerExitedDrawDistance();
    }
    
}

void UAgoraWidgetComponent::SetupOwner()
{
    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    UAgoraUserWidget* WidgetAsUIUserWidget = Cast<UAgoraUserWidget>(Widget);
#if !UE_BUILD_SHIPPING
    if (!WidgetAsUIUserWidget && Widget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Widget in WidgetComponent for %s is not of type AgoraUserWidget. Prefer using AgoraUserWidget for automatic observing functionality."), *GetNameSafe(GetOwner()));
    }
#endif

    if (WidgetAsUIUserWidget)
    {
        WidgetAsUIUserWidget->SetObserved(GetOwner());
    }

    FScriptDelegate DeathDelegate;
    DeathDelegate.BindUFunction(this, "NotifyOwningActorDied");
    IDamageReceiverInterface* OwnerAsDamageReceiver = Cast<IDamageReceiverInterface>(GetOwner());
    if (OwnerAsDamageReceiver)
    {
        OwnerAsDamageReceiver->GetDeathDelegate().AddUnique(DeathDelegate);
    }

    FScriptDelegate RespawnedDelegate;
    RespawnedDelegate.BindUFunction(this, "NotifyOwningActorRespawned");
    AAgoraCharacterBase* OwnerAsCharacterBase = Cast<AAgoraCharacterBase>(GetOwner());
    if (OwnerAsCharacterBase)
    {
        OwnerAsCharacterBase->OnRespawned.AddUnique(RespawnedDelegate);
    }

    FScriptDelegate HiddenDelegate;
    HiddenDelegate.BindUFunction(this, "NotifyOwningActorHidden");
    FScriptDelegate VisibleDelegate;
    VisibleDelegate.BindUFunction(this, "NotifyOwningActorRevealed");
    IVisionInterface* OwnerAsVisionInterface = Cast<IVisionInterface>(GetOwner());
    if (OwnerAsVisionInterface)
    {
        AActor* PlayerControllerViewTarget = UAgoraBlueprintFunctionLibrary::GetViewingPlayerController(GetWorld()) ? UAgoraBlueprintFunctionLibrary::GetViewingPlayerController(GetWorld())->GetViewTarget() : nullptr;
        OwnerAsVisionInterface->GetBecameHiddenDelegate().AddUnique(HiddenDelegate);
        OwnerAsVisionInterface->GetBecameVisibleDelegate().AddUnique(VisibleDelegate);
        if (UAgoraVisionLibrary::LocalPlayerHasVisionOf(GetWorld(), GetOwner()) && DrawBehavior == EDrawBehavior::Visible)
        {
            NotifyOwningActorRevealed();
        }
        else if (UAgoraVisionLibrary::LocalPlayerHasVisionOf(GetWorld(), GetOwner()) && GetOwner()->IsOverlappingActor(PlayerControllerViewTarget) && DrawBehavior == EDrawBehavior::VisibleAndLocalPlayerNear)
        {
            bLocalPlayerIsOverlapped = true;
            NotifyOwningActorRevealed();
        }
        else
        {
            NotifyOwningActorHidden();
        }
    }
  
}

void UAgoraWidgetComponent::NotifyLocalPlayerEnteredDrawDistance()
{
    bLocalPlayerIsOverlapped = true;
    if (UAgoraVisionLibrary::LocalPlayerHasVisionOf(GetWorld(), GetOwner()) && DrawBehavior == EDrawBehavior::VisibleAndLocalPlayerNear)
    {
        if (Widget)
        {
            Widget->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void UAgoraWidgetComponent::NotifyLocalPlayerExitedDrawDistance()
{
    bLocalPlayerIsOverlapped = false;
    if (DrawBehavior == EDrawBehavior::VisibleAndLocalPlayerNear)
    {
        if (Widget)
        {
            Widget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UAgoraWidgetComponent::NotifyOwningActorDied(const FDamageContext& KillingBlow)
{
    if (Widget)
    {
        Widget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UAgoraWidgetComponent::NotifyOwningActorRespawned()
{
    if (Widget && UAgoraVisionLibrary::LocalPlayerHasVisionOf(GetWorld(),GetOwner()))
    {
        Widget->SetVisibility(ESlateVisibility::Visible);
    }
}

void UAgoraWidgetComponent::NotifyOwningActorRevealed()
{
    if (GetOwner()->HasAuthority()) { return; }
    if (DrawBehavior == EDrawBehavior::VisibleAndLocalPlayerNear && !bLocalPlayerIsOverlapped) { return; }

    if (Widget)
    {
        Widget->SetVisibility(ESlateVisibility::Visible);
    }
}

void UAgoraWidgetComponent::NotifyOwningActorHidden()
{
    if (GetOwner()->HasAuthority()) { return; }

    
    if (Widget)
    {
        Widget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

bool UAgoraWidgetComponent::DetermineDistanceToLocalPlayer(float& OutDistance)
{
    APlayerController* ViewingPC = UAgoraBlueprintFunctionLibrary::GetViewingPlayerController(GetWorld());
    AActor* PlayerControllerViewTarget = ViewingPC ? ViewingPC->GetViewTarget() : nullptr;
    if (PlayerControllerViewTarget)
    {
        OutDistance = GetOwner()->GetDistanceTo(PlayerControllerViewTarget);
        return true;
    }
    return false;
}

