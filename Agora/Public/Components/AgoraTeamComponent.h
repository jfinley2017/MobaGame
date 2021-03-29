// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/TeamInterface.h"
#include "AgoraTeamComponent.generated.h"

/**
 * Component given to actors which would like a team. Handles registering and unregistering the owner from 
 * the team service.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UAgoraTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAgoraTeamComponent();

    // UActorComponent
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    // ~UActorComponent

    /**
     * Broadcasted when the owner changes team.
     */
    UPROPERTY(BlueprintAssignable, Category = "AgoraTeamComponent")
    FOnTeamChangedSignature OnTeamChanged;

    // UActorComponent interface
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void InitializeComponent() override;
    // ~UActorComponent interface
	
    /**
     * Sets the team for this component. Also notifys the TeamService that a the owning actor's team has changed.
     */
    UFUNCTION()
    void SetTeam(ETeam NewTeamID);

    /**
     * Returns @TeamID
     */
    UFUNCTION()
    ETeam GetTeam();


protected:

    // UActorComponent interface
    virtual void BeginPlay() override;
   
    // ~UActorComponent interface

    /**
     * Cached TeamID. Saves us an iteration through the TeamService's storage
     */
    UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_TeamID,  Category = "Agora")
    ETeam TeamID = ETeam::Invalid;

    UFUNCTION()
    virtual void OnRep_TeamID();

};
