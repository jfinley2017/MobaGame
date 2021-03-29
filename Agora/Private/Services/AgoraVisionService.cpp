// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraVisionService.h"
#include "AgoraGameplayTags.h"
#include "VisionInterface.h"
#include "AgoraVisionLibrary.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

DECLARE_CYCLE_STAT(TEXT("AddToVisionList"), STAT_AddToVisionList, STATGROUP_AgoraVision);
DECLARE_CYCLE_STAT(TEXT("RemoveFromVisionList"), STAT_RemoveFromVisionList, STATGROUP_AgoraVision);
DECLARE_CYCLE_STAT(TEXT("QueryIsVisibleFor"), STAT_QueryIsVisibleFor, STATGROUP_AgoraVision);
DECLARE_CYCLE_STAT(TEXT("RecalculateVisionForLocalPlayer"), STAT_RecalculateVisionForLocalPlayer, STATGROUP_AgoraVision);

AAgoraVisionService::AAgoraVisionService()
{
    SetReplicates(true);
    bAlwaysRelevant = true;
}

void AAgoraVisionService::BeginPlay()
{
    Super::BeginPlay();

    if (GetNetMode() == NM_DedicatedServer) { return; }

    TRACE(AgoraVision, Log, "AgoraVisionService BeginPlay.");

    SetupLocalPlayer();

}

void AAgoraVisionService::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAgoraVisionService, DawnSpottedActors);
    DOREPLIFETIME(AAgoraVisionService, DuskSpottedActors);

}


void AAgoraVisionService::NotifyGainedVision(AActor* Spotter, ETeam SpottingTeam, AActor* Subject)
{
    if (!HasAuthority()) { return; }

    SCOPE_CYCLE_COUNTER(STAT_AddToVisionList)

    if (SpottingTeam == ETeam::Invalid || SpottingTeam == ETeam::Neutral)
    {
        return;
    }

    if (SpottingTeam == ETeam::Dusk)
    {
        if (DuskViewingReferences.Contains(Subject))
        {
            DuskViewingReferences[Subject].Add(Spotter);
        }
        else
        {
            DuskViewingReferences.Add(Subject, TArray<AActor*>{Spotter});
            DuskSpottedActors.AddUnique(Subject);
            

        }
    }

    if (SpottingTeam == ETeam::Dawn)
    {
        if (DawnViewingReferences.Contains(Subject))
        {
            DawnViewingReferences[Subject].Add(Spotter);
        }
        else
        {
            DawnViewingReferences.Add(Subject, TArray<AActor*>{Spotter});
            DawnSpottedActors.AddUnique(Subject);
            

        }
    }
    
}

void AAgoraVisionService::NotifyLostVision(AActor* Spotter, ETeam SpottingTeam, AActor* Subject)
{
    if (!HasAuthority()) { return; }

    SCOPE_CYCLE_COUNTER(STAT_RemoveFromVisionList)

    if (SpottingTeam == ETeam::Dusk)
    {
        bool DuskViewingReferencesContains = DuskViewingReferences.Contains(Subject);

        if (DuskViewingReferencesContains && DuskViewingReferences[Subject].Num() == 1)
        {
            DuskViewingReferences.Remove(Subject);
            DuskSpottedActors.Remove(Subject);
           
        }
        else if(DuskViewingReferencesContains)
        {
            DuskViewingReferences[Subject].RemoveSingle(Spotter);
        }
    }

    if (SpottingTeam == ETeam::Dawn)
    {
        bool DawnViewingReferencesContains = DawnViewingReferences.Contains(Subject);

        if (DawnViewingReferencesContains && DawnViewingReferences[Subject].Num() == 1)
        {
            DawnViewingReferences.Remove(Subject);
            DawnSpottedActors.Remove(Subject);

        }
        else if (DawnViewingReferencesContains)
        {
            DawnViewingReferences[Subject].RemoveSingle(Spotter);
        }
    }

    // invalid team
    return;
}


bool AAgoraVisionService::IsVisibleFor(const AActor* Viewer, const AActor* Subject)
{
    SCOPE_CYCLE_COUNTER(STAT_QueryIsVisibleFor);

    if(UAgoraGameplayTags::IsSpectator(Viewer))
    {
        return true;
    }
    
    const ITeamInterface* SubjectAsTeamInterface = Cast<ITeamInterface>(Subject);
    const ITeamInterface* ViewerAsTeamInterface = Cast<ITeamInterface>(Viewer);
    if (!ViewerAsTeamInterface || !SubjectAsTeamInterface)
    {
        return true;
    }

    // We have vision if we're on the same team.
    if (SubjectAsTeamInterface->GetTeam() == ViewerAsTeamInterface->GetTeam())
    {
        return true;
    }

    // Else, we only have vision if our team has vision
    ETeam ViewerTeam = ViewerAsTeamInterface->GetTeam();
    return IsVisibleForTeam(ViewerTeam, Subject);

}

bool AAgoraVisionService::IsVisibleForTeam(ETeam Team, const AActor* Subject)
{
    if (Team == ETeam::Dawn)
    {
        return DawnSpottedActors.Contains(Subject);
    }

    if (Team == ETeam::Dusk)
    {
        return DuskSpottedActors.Contains(Subject);
    }

    return false;
}

void AAgoraVisionService::GenerateDebugString(FString& OutDebugString)
{
    OutDebugString = "";

    OutDebugString += "\nDusk:\n";
    for (FAgoraVisibleActor& SpottedActor : DuskSpottedActors.Items)
    {
        FString NumSpotting = DuskViewingReferences.Contains(SpottedActor.Actor) ? FString::FromInt(DuskViewingReferences[SpottedActor.Actor].Num()) : "Empty";
        OutDebugString += *GetNameSafe(SpottedActor.Actor) + FString(" (" + NumSpotting + ")") + FString(",");
    }
    OutDebugString += "\n";

    OutDebugString += "Dawn:\n";
    for (FAgoraVisibleActor& SpottedActor : DawnSpottedActors.Items)
    {
        FString NumSpotting = DawnViewingReferences.Contains(SpottedActor.Actor) ? FString::FromInt(DawnViewingReferences[SpottedActor.Actor].Num()) : "Empty";
        OutDebugString += *GetNameSafe(SpottedActor.Actor) + FString(" (" + NumSpotting + ")") + FString(",");
    }

}

void AAgoraVisionService::NotifyLocalPlayerTeamChanged(AActor* LocalPlayer)
{
    if (GetNetMode() == ENetMode::NM_DedicatedServer) { return; }

    RecalculateVisiblityForAll();
}

void AAgoraVisionService::RecalculateVisiblityForAll()
{
    if (GetNetMode() == ENetMode::NM_DedicatedServer) { return; }

    SCOPE_CYCLE_COUNTER(STAT_RecalculateVisionForLocalPlayer)

    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        IVisionInterface* ActorAsVisionInterface = Cast<IVisionInterface>(*ActorItr);
        if (!ActorAsVisionInterface) { continue; }
        
        if (UAgoraVisionLibrary::LocalPlayerHasVisionOf(GetWorld(), *ActorItr))
        {
            ActorAsVisionInterface->NotifyRevealed(nullptr);
        }
        else
        {
            ActorAsVisionInterface->NotifyRevealedEnd(nullptr);
        }
    }
}


void AAgoraVisionService::SetupLocalPlayer()
{
    static int32 NumFailedSetupFrames = 0;

    APlayerController* LocalPlayerController = UAgoraBlueprintFunctionLibrary::GetViewingPlayerController(GetWorld());
    if (LocalPlayerController)
    {
        ITeamInterface* LocalPlayerControllerAsTeamInterface = Cast<ITeamInterface>(LocalPlayerController);
        if (!LocalPlayerControllerAsTeamInterface)
        {
            NumFailedSetupFrames++;
            TRACE(AgoraVision, Warning, "Local player %s failed to implement ITeamInterface. Cannot track team changes. Trying again next frame. %d failures.", *GetNameSafe(LocalPlayerController), NumFailedSetupFrames);
            GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AAgoraVisionService::SetupLocalPlayer);
            return;
        }

        if (LocalPlayerControllerAsTeamInterface)
        {
            TRACE(AgoraVision, Log, "Local player %s setup in %d frames.", *GetNameSafe(LocalPlayerController), NumFailedSetupFrames);
            FScriptDelegate TeamChangedDelegate;
            TeamChangedDelegate.BindUFunction(this, "NotifyLocalPlayerTeamChanged");
            LocalPlayerControllerAsTeamInterface->GetTeamChangedDelegate().AddUnique(TeamChangedDelegate);
            RecalculateVisiblityForAll();
            return;
        }
    }

    NumFailedSetupFrames++;
    TRACE(AgoraVision, Warning, "Player Controller not valid. Trying again next frame. %d failures.", NumFailedSetupFrames);
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AAgoraVisionService::SetupLocalPlayer);

}

//////////////////////////////////////////////////////////////////////////
// Structs
//////////////////////////////////////////////////////////////////////////

void FAgoraVisibleActor::PostReplicatedAdd(const struct FAgoraVisibleActorsArray& InArraySerializer)
{
    IVisionInterface* RevealedAsVisionInterface = Cast<IVisionInterface>(Actor);

    if (UAgoraVisionLibrary::LocalPlayerHasVisionOf(Actor->GetWorld(), Actor) && RevealedAsVisionInterface)
    {
        // Clients don't know what is revealing them at the moment, just pass in nullptr here
        RevealedAsVisionInterface->NotifyRevealed(nullptr);
    }
}

void FAgoraVisibleActor::PreReplicatedRemove(const struct FAgoraVisibleActorsArray& InArraySerializer)
{
    AActor* CachedActor = Actor;

    // Remove this actor so that we don't pass a team check because of it, this is called before the item
    // is actually removed
    Actor = nullptr;

    IVisionInterface* RevealedAsVisionInterface = Cast<IVisionInterface>(CachedActor);
    
    if (RevealedAsVisionInterface && !UAgoraVisionLibrary::LocalPlayerHasVisionOf(CachedActor->GetWorld(), CachedActor))
    {
        RevealedAsVisionInterface->NotifyRevealedEnd(nullptr);
    }
    
}

bool FAgoraVisibleActorsArray::Contains(const AActor* Actor)
{
    for (const FAgoraVisibleActor& VisibleActor : Items)
    {
        if (VisibleActor.Actor == Actor)
        {
            return true;
        }
    }
    return false;
}

void FAgoraVisibleActorsArray::Remove(const AActor* Actor)
{
    for (int i = 0; i < Items.Num(); i++)
    {
        if (Items[i].Actor == Actor)
        {
            Items.RemoveAt(i);
            MarkArrayDirty();
            return;
        }
    }

    return;

}

void FAgoraVisibleActorsArray::AddUnique(AActor* Actor)
{
    if (!Contains(Actor))
    {
        FAgoraVisibleActor VisibleActor;
        VisibleActor.Actor = Actor;
        int32 AddedIdx = Items.Add(VisibleActor);
        MarkItemDirty(Items[AddedIdx]);
    }
}
