// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraEntityLibrary.h"
#include "AgoraCharacterBase.h"
#include "AgoraGameplayTags.h"
#include "AIModule/Classes/AIController.h"
#include "AgoraSceneTargetComponent.h"
#include "GameFramework/PlayerController.h"
#include "GASCharacterMovementComponent.h"
#include "BrainComponent.h"
#include "DamageReceiverInterface.h"
#include "AgoraHeroBase.h"

DEFINE_LOG_CATEGORY(AgoraEntity);

UTexture* UAgoraEntityLibrary::GetDisplayIcon(AActor* Actor)
{
    AAgoraCharacterBase* ActorAsCharacter = Cast<AAgoraCharacterBase>(Actor);
    if (ActorAsCharacter)
    {
        return ActorAsCharacter->CharacterMinimapIcon;
    }
    TRACESTATIC(AgoraEntity, Warning, "%s is not of type AAgoraCharacterBase. Cannot retreive display icon.", *GetNameSafe(Actor));
    return nullptr;
}

bool UAgoraEntityLibrary::IsDead(AActor* Actor)
{
    IDamageReceiverInterface* ActorAsDamageReceiver = Cast<IDamageReceiverInterface>(Actor);
    if (ActorAsDamageReceiver)
    {
        return ActorAsDamageReceiver->IsDead();
    }
    TRACESTATIC(AgoraEntity, Warning, "%s is not of type IDamageReceiverInterface. Cannot determine if dead.", *GetNameSafe(Actor));
    return false;
}

bool UAgoraEntityLibrary::GetRemainingRespawnDuration(AActor* Actor, float& OutRespawnTime)
{
    AAgoraHeroBase* ActorAsHero = Cast<AAgoraHeroBase>(Actor);
    if (ActorAsHero)
    {
        float CurrentRespawnTimestamp = 0.0f;
        if (ActorAsHero->GetRespawnTime(CurrentRespawnTimestamp))
        {
            OutRespawnTime = CurrentRespawnTimestamp - UAgoraBlueprintFunctionLibrary::GetTimeSinceStartOfGame(Actor->GetWorld());
            return true;
        }
        return false;
    }
    TRACESTATIC(AgoraEntity, Warning, "%s is not of type AAgoraHeroBase. Cannot determine respawn duration.", *GetNameSafe(Actor));
    return false;
}

bool UAgoraEntityLibrary::GetRespawnTimestamp(AActor* Actor, float& OutRespawnTimestamp)
{
    AAgoraHeroBase* ActorAsHero = Cast<AAgoraHeroBase>(Actor);
    if (ActorAsHero)
    {
        return ActorAsHero->GetRespawnTime(OutRespawnTimestamp);
    }
    TRACESTATIC(AgoraEntity, Warning, "%s is not of type AAgoraHeroBase. Cannot determine respawn duration.", *GetNameSafe(Actor));
    return false;
}

//void UAgoraEntityLibrary::GetAllPawns(UObject* WorldContextObject, TArray<APawn*>& OutPawns, AActor* ReferenceActor, const FAgoraSweepQueryParams& QueryParams)
//{
//    if (!WorldContextObject || !WorldContextObject->ImplementsGetWorld())
//    {
//        TRACESTATIC(Agora, Error, "WorldContextObject: %s doesn't exist or is invalid.", *GetNameSafe(WorldContextObject));
//        return;
//    }
//
//    for (FConstPawnIterator Iterator = WorldContextObject->GetWorld()->GetPawnIterator(); Iterator; Iterator++)
//    {
//        if (QueryParams.MatchesQuery(ReferenceActor, Iterator->Get()))
//        {
//            OutPawns.Push(Iterator->Get());
//        }
//    }
//}

void UAgoraEntityLibrary::CompletelyDisableInput(APawn* PawnToDisable)
{
    if (!PawnToDisable) { return; }

    if (PawnToDisable->IsPlayerControlled())
    {
        PawnToDisable->DisableInput(Cast<APlayerController>(PawnToDisable->GetController()));
        return;
    }
    else if (PawnToDisable->HasAuthority())
    {
        AAIController* Controller = Cast<AAIController>(PawnToDisable->GetController());
        if (Controller && Controller->BrainComponent)
        {
            Controller->BrainComponent->StopLogic("Disabled");
        }
    }
}

void UAgoraEntityLibrary::EnableInput(APawn* PawnToEnable)
{
    if (!PawnToEnable) { return; }

    if (PawnToEnable->IsPlayerControlled())
    {
        PawnToEnable->EnableInput(Cast<APlayerController>(PawnToEnable->GetController()));
        return;
    }
    else if (PawnToEnable->HasAuthority())
    {
        AAIController* Controller = Cast<AAIController>(PawnToEnable->GetController());
        if (Controller && Controller->BrainComponent)
        {
            Controller->BrainComponent->RestartLogic();
        }
    }
}

EDescriptor UAgoraEntityLibrary::GetEntityType(AActor* Actor)
{
    if (UAgoraGameplayTags::IsHero(Actor)) return EDescriptor::Hero;
    if (UAgoraGameplayTags::IsTower(Actor)) return EDescriptor::Tower;
    if (UAgoraGameplayTags::IsCore(Actor)) return EDescriptor::Core;
    if (UAgoraGameplayTags::IsJungleCreep(Actor)) return EDescriptor::JungleCreep;
    if (UAgoraGameplayTags::IsMinion(Actor)) return EDescriptor::Minion;
    if (UAgoraGameplayTags::IsSpectator(Actor)) return EDescriptor::Spectator;

    return EDescriptor::Invalid;
}


USceneComponent* UAgoraEntityLibrary::GetEntityTargetLocation(AActor* Actor)
{
    UAgoraSceneTargetComponent* TargetComponent = Actor->FindComponentByClass<UAgoraSceneTargetComponent>();
    if (TargetComponent)
    {
        return TargetComponent;
    }
    return Actor->GetRootComponent();
}

USkeletalMeshComponent* UAgoraEntityLibrary::GetMesh(AActor* Actor)
{
    ACharacter* ActorAsCharacter = Cast<ACharacter>(Actor);
    if (ActorAsCharacter)
    {
        return ActorAsCharacter->GetMesh();
    }
    return nullptr;
}

void UAgoraEntityLibrary::DisableMovement(APawn* PawnToDisable)
{
    if (!PawnToDisable) { return; }

    if (PawnToDisable->IsPlayerControlled())
    {
        Cast<APlayerController>(PawnToDisable->GetController())->SetIgnoreMoveInput(true);
        return;
    }
    else if (PawnToDisable->HasAuthority())
    {
        AAIController* Controller = Cast<AAIController>(PawnToDisable->GetController());
        if (Controller && Controller->BrainComponent)
        {
            Controller->SetIgnoreMoveInput(true);
        }
    }
}

void UAgoraEntityLibrary::EnableMovement(APawn* PawnToDisable)
{
    if (!PawnToDisable) { return; }

    if (PawnToDisable->IsPlayerControlled())
    {
        Cast<APlayerController>(PawnToDisable->GetController())->SetIgnoreMoveInput(false);
        return;
    }
    else if (PawnToDisable->HasAuthority())
    {
        AAIController* Controller = Cast<AAIController>(PawnToDisable->GetController());
        if (Controller && Controller->BrainComponent)
        {
            Controller->SetIgnoreMoveInput(false);
        }
    }
}

void UAgoraEntityLibrary::EnableMouseLook(APawn* PawnToEnable)
{
    if (!PawnToEnable) { return; }

    if (PawnToEnable->IsPlayerControlled())
    {
        Cast<APlayerController>(PawnToEnable->GetController())->SetIgnoreLookInput(false);
        return;
    }
}

void UAgoraEntityLibrary::DisableMouseLook(APawn* PawnToDisable)
{
    if (!PawnToDisable) { return; }

    if (PawnToDisable->IsPlayerControlled())
    {
        Cast<APlayerController>(PawnToDisable->GetController())->SetIgnoreLookInput(true);
        return;
    }
}

void UAgoraEntityLibrary::DisableTurnInPlace(AAgoraCharacterBase* Character)
{
    if (Character) Character->GetGASCMC()->bTurnInPlaceEnabled = false;
}

void UAgoraEntityLibrary::EnableTurnInPlace(AAgoraCharacterBase* Character)
{
    if (Character) Character->GetGASCMC()->bTurnInPlaceEnabled = true;
}