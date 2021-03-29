// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/AgoraUserWidget.h"
#include "AgoraAffiliatedWidget.generated.h"

/** What relationship does the owner of this widget (the one drawing us) have to us? */
UENUM(BlueprintType)
enum class EOwnerRelationship : uint8
{
    None, // The observed doesn't exist, or otherwise has no relation to the owner
    Spectator, // The owner is a spectator
    Enemy, // The observed is an enemy
    Friendly, // The observed is friendly
    Neutral, // The observed is neutral
    ObservedOwner // The owner also owns the actor being observed (eg the overhead healthbar of a character we are controlling)
};

/**
 * Extends UAgoraUserWidget to provide functionality based on the relationship between the observed and the observer (the one who is drawing/owning the widget)
 * Useful for team based styling.
 */
UCLASS()
class AGORA_API UAgoraAffiliatedWidget : public UAgoraUserWidget
{
	GENERATED_BODY()
	
public:

    UAgoraAffiliatedWidget(const FObjectInitializer& ObjectInitializer);

    /** Hook to setup the style based on a provided relationship. */
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UIUserWidget")
    void SetupStyle(EOwnerRelationship OwnerRelationship);

    // UAgoraUserWidget interface
    virtual void SetObserved(AActor* InObservedActor) override;
    // ~UAgoraUserWidget interface

protected:

    // UWidget interface
    virtual void NativeConstruct() override;
    // ~UWidget interface

    //////////////////////////////////////////////////////////////////////////
    // Relationship calculation

    UFUNCTION(BlueprintPure, Category = "UIUserWidget")
    EOwnerRelationship DetermineObservedRelationshipToOwner();

    /** Returns whether or not the view (widget owner) is an enemy to this unit. */
    UFUNCTION(BlueprintPure, Category = "UIUserWidget")
    bool IsEnemy();

    /** Returns whether or not the view (widget owner) is a friendly to this unit */
    UFUNCTION(BlueprintPure, Category = "UIUserWidget")
    bool IsFriendly();

    /** Returns the actor we should use to represent the owner (the one viewing this widget). Typically the view target/pawn of the owning player controller. */
    AActor* GetOwnerRepresentative();

    //////////////////////////////////////////////////////////////////////////
    // Design
    
    /** Debug variable used to display style changes in design time. Should be ignored when not in design time. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIUserWidget")
    EOwnerRelationship DesignOwnerRelationship;

    //////////////////////////////////////////////////////////////////////////
    // Team changing

    UFUNCTION()
    void OnTeamChanged(AActor* ChangedActor);

    /** Holds signatures to this widget's OnTeamChanged function, to be used for team changing*/
    FScriptDelegate TeamChangedDelegate;
    FScriptDelegate OwnerTeamChangedDelegate;

  
};
