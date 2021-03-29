// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AgoraAffiliatedWidget.h"
#include "AgoraAffiliationComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelationshipToLocalPlayerChangedSignature, EOwnerRelationship, NewRelationshipToLocalPlayer);

/**
 * Used for actors which need to monitor team changes for the local player. Turret color, core colors, etc.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGORA_API UAgoraAffiliationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

    UPROPERTY(BlueprintAssignable)
    FOnRelationshipToLocalPlayerChangedSignature OnOwnerRelationshipChanged;

	// Sets default values for this component's properties
	UAgoraAffiliationComponent();

    UFUNCTION(BlueprintPure, Category = "AgoraAffiliationComponent")
    EOwnerRelationship GetOwnerRelationshipToLocalPlayer();

    UFUNCTION()
    void NotifyTeamChanged(AActor* ChangedActor);

protected:
    
    // UActorComponent
    virtual void BeginPlay() override;
    // ~UActorComponent

    void SetupLocalPlayer();
    void SetupOwner();
    void RefreshRelationshipToLocalPlayer();

    EOwnerRelationship CachedOwnerRelationshipToLocalPlayer = EOwnerRelationship::None;

    FScriptDelegate TeamChangedDelegate;
    
};
