// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "AgoraTypes.h"
#include "AgoraWidgetComponent.generated.h"

UENUM(BlueprintType)
enum class EDrawBehavior : uint8
{
    Visible,
    VisibleAndLocalPlayerNear
};

/**
 * Overridden for any game-specific base functionality.
 */
UCLASS()
class AGORA_API UAgoraWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

    // UWidgetComponent interface
    virtual void BeginPlay() override;
    virtual void InitWidget() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
    // ~UWidgetComponent interface

    virtual void SetupOwner();

    UFUNCTION()
    void NotifyLocalPlayerEnteredDrawDistance();
    UFUNCTION()
    void NotifyLocalPlayerExitedDrawDistance();

protected:

    UFUNCTION()
    virtual void NotifyOwningActorDied(const FDamageContext& KillingBlow);

    UFUNCTION()
    virtual void NotifyOwningActorRespawned();

    UFUNCTION()
    virtual void NotifyOwningActorRevealed();

    UFUNCTION()
    virtual void NotifyOwningActorHidden();

    UFUNCTION()
    bool DetermineDistanceToLocalPlayer(float& OutDistance);

    /*
     * Draw behavior, when should we draw this widget? 
     */
    UPROPERTY(EditAnywhere, Category = "Agora")
    EDrawBehavior DrawBehavior = EDrawBehavior::Visible;

    /**
     * If we're using EDrawBehavior::VisibleAndLocalPlayerNear, this is the distance where we will display this
     * widget.
     */
    UPROPERTY(EditAnywhere, Category = "Agora")
    float DrawDistance = 1600.0f;

    /** Hacky way to ensure that we are setup correctly and avoid any race conditions*/
    bool bInitialized = false;
    bool bLocalPlayerIsOverlapped = false;
};
