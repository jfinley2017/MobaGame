// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#include "AgoraCheatManager.h"
#include "ConstructorHelpers.h"
#include "UserWidget.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"

#include "Agora.h"
#include "TeamInterface.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraLaneMinion.h"
#include "AgoraHeroBase.h"
#include "AgoraLevelComponent.h"
#include "AgoraGameInstance.h"
#include "AgoraInventoryComponent.h"
#include "AgoraItemLibrary.h"


UAgoraCheatManager::UAgoraCheatManager()
{
	ConstructorHelpers::FClassFinder<AAgoraLaneMinion> MinionFinder(TEXT("/Game/Blueprints/Minions/BP_LaneMinion_Ranged"));
	MinionClass = MinionFinder.Class;
}

void UAgoraCheatManager::AgSlomo(float Dilation)
{
	ServerSlomo(Dilation);
}

void UAgoraCheatManager::ServerSlomo_Implementation(float Dilation)
{
	UGameplayStatics::SetGlobalTimeDilation(GetOuterAPlayerController()->GetWorld(), Dilation);
}

bool UAgoraCheatManager::ServerSlomo_Validate(float Dilation)
{
	return true;
}

void UAgoraCheatManager::ServerToggleTeam_Implementation(APawn* Pawn)
{
	if (GetWorld()->GetNetMode() < NM_Client)
	{
		ITeamInterface* TeamPawn = Cast<ITeamInterface>(Pawn);

		bool TeamInvalidOrDawn = TeamPawn->GetTeam() == ETeam::Invalid || TeamPawn->GetTeam() == ETeam::Dawn;
		
		TeamInvalidOrDawn ? TeamPawn->SetTeam(ETeam::Dusk) : TeamPawn->SetTeam(ETeam::Dawn);
        ETeam PawnTeam = TeamPawn->GetTeam();
		SCREENDEBUG_BLUE(10.0f, "Switched team to %s", *GETENUMSTRING("ETeam",PawnTeam));
	}
}

bool UAgoraCheatManager::ServerToggleTeam_Validate(APawn* Pawn) { return true; }

void UAgoraCheatManager::ServerSetAttribute_Implementation(APawn* Pawn, const FString& AttributeName, float Value)
{
	if (GetWorld()->GetNetMode() < NM_Client)
	{
		IAbilitySystemInterface* AgUnit = Cast<IAbilitySystemInterface>(Pawn);

		if (AgUnit)
		{
			UGameplayEffect* SetAttributeEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("SetAttributeEffect")));

			UAbilitySystemComponent* ASC = AgUnit->GetAbilitySystemComponent();

			FGameplayModifierInfo Info;
			Info.ModifierMagnitude = FScalableFloat(Value);
			Info.ModifierOp = EGameplayModOp::Override;
			Info.Attribute = UAgoraBlueprintFunctionLibrary::GetAttributeFromString(AttributeName, ASC);
			SetAttributeEffect->Modifiers.Add(Info);
			SetAttributeEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

			ASC->ApplyGameplayEffectToSelf(SetAttributeEffect, 1, ASC->MakeEffectContext());
		}
	}
}

bool UAgoraCheatManager::ServerSetAttribute_Validate(APawn* Pawn, const FString& AttributeName, float Value) { return true; }

void UAgoraCheatManager::AgKill()
{
    FHitResult Result;
    if (GetLookLocation(Result))
    {
        ServerSetAttribute(Cast<APawn>(Result.Actor), FString(TEXT("Damage")), 99999);
    }

}

void UAgoraCheatManager::AgGrantItem(const FString& ItemName)
{
    ServerGrantItem(ItemName);
}

void UAgoraCheatManager::AgRemoveItem(const FString& ItemName)
{
    ServerRemoveItem(ItemName);
}

void UAgoraCheatManager::ServerRemoveItem_Implementation(const FString& ItemName)
{
    APawn* AssociatedPawn = GetOuterAPlayerController()->GetPawn();
    UAgoraInventoryComponent* InvComp = AssociatedPawn->FindComponentByClass<UAgoraInventoryComponent>();
    if (InvComp)
    {
        UAgoraItem* ItemToRemove = UAgoraItemLibrary::GetItemByName(GetWorld(),ItemName);
        InvComp->RemoveItem(ItemToRemove,1);
    }
}

bool UAgoraCheatManager::ServerRemoveItem_Validate(const FString& ItemName)
{
    return true;
}

void UAgoraCheatManager::ServerGrantItem_Implementation(const FString& ItemName)
{
    APawn* AssociatedPawn = GetOuterAPlayerController()->GetPawn();
    UAgoraInventoryComponent* InvComp = AssociatedPawn->FindComponentByClass<UAgoraInventoryComponent>();
    if (InvComp)
    {
        UAgoraItem* ItemToAdd = UAgoraItemLibrary::GetItemByName(GetWorld(), ItemName);
        InvComp->EquipItem(ItemToAdd,1);
    }
}

bool UAgoraCheatManager::ServerGrantItem_Validate(const FString& ItemName)
{
    return true;
}

void UAgoraCheatManager::AgGrantItems(const FString& Item1, const FString& Item2, const FString& Item3, const FString& Item4, const FString& Item5, const FString& Item6)
{
    // OuO
    if (Item1 != FString("")) { ServerGrantItem(Item1); };
    if (Item2 != FString("")) { ServerGrantItem(Item2); };
    if (Item3 != FString("")) { ServerGrantItem(Item3); };
    if (Item4 != FString("")) { ServerGrantItem(Item4); };
    if (Item5 != FString("")) { ServerGrantItem(Item5); };
    if (Item6 != FString("")) { ServerGrantItem(Item6); };
}

void UAgoraCheatManager::AgRemoveItems()
{
    APawn* AssociatedPawn = GetOuterAPlayerController()->GetPawn();
    UAgoraInventoryComponent* InvComp = AssociatedPawn->FindComponentByClass<UAgoraInventoryComponent>();
    if (InvComp)
    {
        for (int i = 0; i < 6; i++)
        {
            FAgoraInventorySlot ItemSlot;
            InvComp->GetInventorySlotAt(i, ItemSlot);

            if (!ItemSlot.IsEmpty())
            {
                ServerRemoveItem(ItemSlot.SlottedItem.Item->GetItemName());
            }
        }
    }
}

void UAgoraCheatManager::AgSuicide()
{
	APawn* Pawn = GetOuterAPlayerController()->GetPawn();

    AgSetAttribute(FString("Damage"), 999999);
}

void UAgoraCheatManager::AgSpawnMinion(bool bSameTeam)
{
	APawn* AssociatedPawn = GetOuterAPlayerController()->GetPawn();
    FHitResult Result;
    GetLookLocation(Result);
	ServerSpawnMinion(AssociatedPawn, Result.Location, bSameTeam);
}

void UAgoraCheatManager::AgIncreaseLevel()
{
	ServerIncreaseLevel();
}

void UAgoraCheatManager::ServerIncreaseLevel_Implementation()
{
	APawn* Pawn = GetOuterAPlayerController()->GetPawn();
	AAgoraHeroBase* AgoraHero = Cast<AAgoraHeroBase>(Pawn);
	if (AgoraHero)
	{
		FXpLevelUpData* XpData = AgoraHero->GetLevelComponent()->GetTotalXpForNextLevel();
		if (XpData)
		{
			AgSetAttribute("Experience", XpData->XpToLevel);
		}
	}
}

bool UAgoraCheatManager::ServerIncreaseLevel_Validate()
{
	return true;
}

void UAgoraCheatManager::ServerSpawnMinion_Implementation(APawn* Pawn, FVector TargetLocation, bool bSameTeam)
{
	AAgoraLaneMinion* Minion = GetWorld()->SpawnActorDeferred<AAgoraLaneMinion>(MinionClass, FTransform(FRotator::ZeroRotator, TargetLocation,  FVector::OneVector));

	TScriptInterface<ITeamInterface> PawnTeam = Pawn;

	ETeam TeamToUse = PawnTeam->GetTeam();

	if (!bSameTeam)
	{
		TeamToUse = TeamToUse == ETeam::Dawn ? ETeam::Dusk : ETeam::Dawn;
	}

	Minion->SetTeam(TeamToUse);
	Minion->FinishSpawning(Minion->GetActorTransform());
}

bool UAgoraCheatManager::ServerSpawnMinion_Validate(APawn* Pawn, FVector TargetLocation, bool bSameTeam)
{
	return true;
}

void UAgoraCheatManager::AgGodMode()
{
	AgSetAttribute("MovementSpeedBase", 1000);
	AgSetAttribute("MagicalPower", 9999);
	AgSetAttribute("PhysicalPower", 9999);
	AgSetAttribute("CleavePercentage", 1.0);
	AgSetAttribute("ManaRegen", 9999);
	AgSetAttribute("HealthRegen", 1000);
	AgSetAttribute("CooldownReduction", 0.3);
	AgSetAttribute("Experience", 10000);
}

void UAgoraCheatManager::AgSetAttribute(const FString& AttributeName, float Value)
{
	APawn* Pawn = GetOuterAPlayerController()->GetPawn();

	ServerSetAttribute(Pawn, AttributeName, Value);
}

void UAgoraCheatManager::AgToggleTeam()
{
	APawn* Pawn = GetOuterAPlayerController()->GetPawn();

	ServerToggleTeam(Pawn);
}

void UAgoraCheatManager::ServerAgTeleport_Implementation(APawn* Pawn, FVector TargetLocation)
{
	Pawn->TeleportTo(TargetLocation, Pawn->GetActorRotation());
}

bool UAgoraCheatManager::ServerAgTeleport_Validate(APawn* Pawn, FVector TargetLocation)
{
	return true;
}

// Implementation derived from CheatManager
void UAgoraCheatManager::AgTeleport()
{
	APawn* AssociatedPawn = GetOuterAPlayerController()->GetPawn();
    FHitResult Result;
    GetLookLocation(Result);
	ServerAgTeleport(AssociatedPawn, Result.Location);
}

bool UAgoraCheatManager::GetLookLocation(FHitResult& Result)
{
	FVector	ViewLocation;
	FRotator ViewRotation;
	check(GetOuterAPlayerController() != nullptr);
	GetOuterAPlayerController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	APawn* AssociatedPawn = GetOuterAPlayerController()->GetPawn();
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(TeleportTrace), true, AssociatedPawn);

    return GetWorld()->LineTraceSingleByChannel(Result, ViewLocation, ViewLocation + 1000000.f * ViewRotation.Vector(), ECC_Pawn, TraceParams);
}
