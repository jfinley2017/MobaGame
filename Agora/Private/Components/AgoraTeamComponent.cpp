// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraTeamComponent.h"
#include "AgoraTeamLibrary.h"
#include "Agora.h"
#include "Net/UnrealNetwork.h"
#include "AgoraTeamService.h"

// Sets default values for this component's properties
UAgoraTeamComponent::UAgoraTeamComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
    bReplicates = true;
    bWantsInitializeComponent = true;
	// ...
}

void UAgoraTeamComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAgoraTeamComponent, TeamID);

}

// Called when the game starts
void UAgoraTeamComponent::BeginPlay()
{
	Super::BeginPlay();

    if (GetTeam() == ETeam::Invalid && !GetWorld()->IsEditorWorld())
    {
        TRACE(AgoraTeam, Warning, "%s BeginPlay with invalid team.", *GetNameSafe(GetOwner()));
    }
  
    
}

void UAgoraTeamComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    AAgoraTeamService* TeamService = UAgoraTeamLibrary::GetAgoraTeamService(this);
    if (GetTeam() != ETeam::Invalid && TeamService)
    {
        TRACE(AgoraTeam, Log, "%s EndPlay with valid team. Removing", *GetNameSafe(GetOwner()));
        TeamService->RemoveActorFromTeam(GetTeam(), GetOwner());
    }

    Super::EndPlay(EndPlayReason);
}

// Called every frame
void UAgoraTeamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

void UAgoraTeamComponent::SetTeam(ETeam NewTeamID)
{
    if (!GetOwner()->HasAuthority()) { return; }

    if (NewTeamID == TeamID) { return; }

    TeamID = NewTeamID;
    OnRep_TeamID();

    AAgoraTeamService* TeamService = UAgoraTeamLibrary::GetAgoraTeamService(this);
    if (!TeamService) 
    { 
        TRACE(AgoraTeam, Warning, "Team set on %s, but team service could not be found. Cannot notify service of team state.", *GetNameSafe(GetOwner()));
        return;
    }

    if (TeamID != ETeam::Invalid)
    {
        TeamService->RemoveActorFromTeam(TeamID, GetOwner());
    }

    TeamService->AddActorToTeam(TeamID, GetOwner());

}

ETeam UAgoraTeamComponent::GetTeam()
{
    return TeamID;
}

void UAgoraTeamComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (!GetOwner()->HasAuthority())
    {
        return;
    }

    if (TeamID != ETeam::Invalid)
    {
        // hack to force ourselves to register with our team on BeginPlay, if our team was set by EditDefaultsOnly
        ETeam CacheTeamValue = TeamID;
        TeamID = ETeam::Invalid;
        SetTeam(CacheTeamValue);
    }

}

void UAgoraTeamComponent::OnRep_TeamID()
{
    OnTeamChanged.Broadcast(GetOwner());
}

