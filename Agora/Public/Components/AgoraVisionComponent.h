// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraVisionLibrary.h"
#include "Components/SphereComponent.h"
#include "Interfaces/VisionInterface.h"
#include "Interfaces/TeamInterface.h"
#include "GameplayTagContainer.h"
#include "AgoraVisionComponent.generated.h"

class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraVisionComponent : public USphereComponent
{
	GENERATED_BODY()
	
public:

    FOnVisibilityToggledSignature OnRevealed;
    FOnVisibilityToggledSignature OnHidden;
    FOnVisionLevelChangedSignature OnVisionLevelChanged;
    FOnHiddenLevelChangedSignature OnHiddenLevelChanged;

    UAgoraVisionComponent();

    // UActorComponent interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
    // ~UActorComponent interface

    // to be removed, need another component for this
    virtual void NotifyRevealed(AActor* RevealingActor);
    virtual void NotifyEndRevealed(AActor* EndRevealingActor);

    UFUNCTION()
    virtual void NotifyVisionServiceReady(AAgoraVisionService* VisionService);

    UFUNCTION()
    virtual void NotifyOwnerTeamChanged(AActor* ChangedActor);

    virtual uint8 GetVisionLevel() const;
    virtual uint8 GetHiddenLevel() const;

protected:

    UFUNCTION()
    void NotifyOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
    UFUNCTION()
    void NotifyEndOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    
    void NotifyNewSpottedActor(AActor* SpottedActor);
    void NotifyRemoveSpottedActor(AActor* RemovedSpottedActor);
    
    void SetupOwner();
    void SetupCollision();

    bool OwnerHasLineOfSightTo(AActor* Actor);
    void UpdateOverlappingVision(ETeam TeamToGrantVisionTo, ETeam TeamToRemoveVisionFrom);

    /** Should we apply vision to this actor? Good place to do line of sight checks for example */
    bool ShouldApplyVisionTo(AActor* Actor, bool bSkipLosCheck);

    uint8 CurrentVisionLevel = 0;
    uint8 CurrentHiddenLevel = 0;
    TArray<AActor*> CurrentlySpottedActors;
    ITeamInterface* OwnerAsTeamInterface;
    ETeam CurrentTeam;
    ETeam PreviousTeam;

    // to be removed
    bool bIsOwnerVisible = false;
    bool bHasBeenPassedOnce = false;

};
