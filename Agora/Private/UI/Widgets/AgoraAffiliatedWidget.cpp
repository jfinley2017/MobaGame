// Fill out your copyright notice in the Description page of Project Settings.

#include "AgoraAffiliatedWidget.h"
#include "TeamInterface.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraGameplayTags.h"

UAgoraAffiliatedWidget::UAgoraAffiliatedWidget(const FObjectInitializer& ObjectInitializer)
{
    TeamChangedDelegate.BindUFunction(this, "OnTeamChanged");
    OwnerTeamChangedDelegate.BindUFunction(this, "OnTeamChanged");
}

void UAgoraAffiliatedWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (GetOwnerRepresentative())
    {
        ITeamInterface* TeamReferenceActorAsTeamInterface = Cast<ITeamInterface>(GetOwnerRepresentative());
        ensure(TeamReferenceActorAsTeamInterface);
        if (TeamReferenceActorAsTeamInterface)
        {
            TeamReferenceActorAsTeamInterface->GetTeamChangedDelegate().AddUnique(OwnerTeamChangedDelegate);
        }
    }

}

void UAgoraAffiliatedWidget::SetObserved(AActor* InObservedActor)
{
    AActor* OldObserved = Observed;

    if (OldObserved)
    {
        ITeamInterface* OldObservedAsTeamInterface = Cast<ITeamInterface>(OldObserved);
        if (OldObservedAsTeamInterface)
        {
            OldObservedAsTeamInterface->GetTeamChangedDelegate().Remove(TeamChangedDelegate);
        }
    }

    Super::SetObserved(InObservedActor);

    if (Observed)
    {
        ITeamInterface* ObservedAsTeamInterface = Cast<ITeamInterface>(Observed);
        if (ObservedAsTeamInterface)
        {
            ObservedAsTeamInterface->GetTeamChangedDelegate().AddUnique(TeamChangedDelegate);
        }
    }

}

EOwnerRelationship UAgoraAffiliatedWidget::DetermineObservedRelationshipToOwner()
{
    if (!Observed)
    {
        return EOwnerRelationship::None;
    }
	
	if (UAgoraGameplayTags::IsSpectator(GetOwnerRepresentative()))
	{
		return EOwnerRelationship::Spectator;
	}
    
    if (GetOwningPlayerPawn() == Observed)
    {
        return EOwnerRelationship::ObservedOwner;
    }

    if (IsEnemy())
    {
        return EOwnerRelationship::Enemy;
    }

    if (IsFriendly())
    {
        return EOwnerRelationship::Friendly;
    }


    return EOwnerRelationship::None;
}

AActor* UAgoraAffiliatedWidget::GetOwnerRepresentative()
{
    return GetOwningPlayer();
}

bool UAgoraAffiliatedWidget::IsEnemy()
{
    return !(UAgoraBlueprintFunctionLibrary::IsFriendly(Observed, GetOwnerRepresentative()));
}

bool UAgoraAffiliatedWidget::IsFriendly()
{
    return UAgoraBlueprintFunctionLibrary::IsFriendly(Observed,GetOwnerRepresentative());
}

void UAgoraAffiliatedWidget::OnTeamChanged(AActor* ChangedActor)
{
    // Refresh the style
    //if (GetOwningPlayer()->HasAuthority()) { return; }
    SetupStyle(DetermineObservedRelationshipToOwner());
}
