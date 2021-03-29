// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraAffiliationComponent.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraTeamLibrary.h"

// Sets default values for this component's properties
UAgoraAffiliationComponent::UAgoraAffiliationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;


    TeamChangedDelegate.BindUFunction(this, "NotifyTeamChanged");
	// ...
}


// Called when the game starts
void UAgoraAffiliationComponent::BeginPlay()
{
	Super::BeginPlay();

    if (GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    SetupLocalPlayer();
    SetupOwner();
}


void UAgoraAffiliationComponent::SetupLocalPlayer()
{
    APlayerController* LocalPlayerController = UAgoraBlueprintFunctionLibrary::GetViewingPlayerController(GetWorld());
    if (LocalPlayerController)
    {
        ITeamInterface* LocalPlayerAsTeamInterface = Cast<ITeamInterface>(LocalPlayerController);
        if (LocalPlayerAsTeamInterface)
        {
            LocalPlayerAsTeamInterface->GetTeamChangedDelegate().AddUnique(TeamChangedDelegate);
        }
        TRACE(LogAgoraStartup, Warning, "%s did not implement ITeamInterface. Cannot determine affiliation.", *GetNameSafe(LocalPlayerController));
    }
    TRACE(LogAgoraStartup, Warning, "Could not find local player controller, cannot determine affiliation.");
}

void UAgoraAffiliationComponent::SetupOwner()
{
    ITeamInterface* OwnerAsTeamInterface = Cast<ITeamInterface>(GetOwner());
    if (OwnerAsTeamInterface)
    {
        OwnerAsTeamInterface->GetTeamChangedDelegate().AddUnique(TeamChangedDelegate);
    }
    TRACE(LogAgoraStartup, Warning, "%s did not implement ITeamInterface. Cannot determine affiliation.", *GetNameSafe(GetOwner()));
}

void UAgoraAffiliationComponent::RefreshRelationshipToLocalPlayer()
{
    EOwnerRelationship NewOwnerRelationship = UAgoraTeamLibrary::DetermineRelationshipToLocalPlayer(GetWorld(), GetOwner());
    if (NewOwnerRelationship == CachedOwnerRelationshipToLocalPlayer)
    {
        return;
    }

    CachedOwnerRelationshipToLocalPlayer = NewOwnerRelationship;
    OnOwnerRelationshipChanged.Broadcast(CachedOwnerRelationshipToLocalPlayer);

}

EOwnerRelationship UAgoraAffiliationComponent::GetOwnerRelationshipToLocalPlayer()
{
    return CachedOwnerRelationshipToLocalPlayer;
}

void UAgoraAffiliationComponent::NotifyTeamChanged(AActor* ChangedActor)
{
    RefreshRelationshipToLocalPlayer();
}

