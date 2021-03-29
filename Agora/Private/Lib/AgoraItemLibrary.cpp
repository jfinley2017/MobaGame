// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraItemLibrary.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraInventoryComponent.h"
#include "AgoraDataSingleton.h"
#include "AgoraGameState.h"
#include "AgoraItemService.h"
#include "Agora.h"

DEFINE_LOG_CATEGORY(AgoraItem);

const FPrimaryAssetType UAgoraItemLibrary::AgoraItemAssetType = TEXT("AgoraItem");


AAgoraItemService* UAgoraItemLibrary::GetItemService(UObject* WorldContextObject)
{
    AAgoraGameState* GameState = WorldContextObject->GetWorld()->GetGameState<AAgoraGameState>();
    if (!GameState)
    {
        static bool bPrintWrongGSError = true;
        if (bPrintWrongGSError)
        {
            TRACESTATIC(AgoraItem, Error, "AgoraGameState not found. Could not reference ItemService.")
        }
        return nullptr;
    }
    return GameState->GetItemService();
}

UAgoraInventoryComponent* UAgoraItemLibrary::GetInventoryComponent(AActor* Actor)
{
	if (!Actor) { return nullptr; }

    return Actor->FindComponentByClass<UAgoraInventoryComponent>();
}

void UAgoraItemLibrary::GetItems(UObject* WorldContextObject, TArray<UAgoraItem*>& Items)
{
    AAgoraItemService* ItemService = GetItemService(WorldContextObject);
    if (ItemService)
    {
        return ItemService->GetItems(Items);
    }
}

UAgoraItem* UAgoraItemLibrary::GetItemByName(UObject* WorldContextObject, const FString& ItemName)
{
    AAgoraItemService* ItemService = GetItemService(WorldContextObject);
    if (ItemService)
    {
        UAgoraItem* FoundItem = ItemService->GetItemFromName(ItemName);
        //#TODO move logging into item service
        if (!FoundItem) { TRACESTATIC(AgoraItem, Warning, "Attempted to get item %s but an item with name %s did not exist.", *ItemName, *ItemName) }
        return FoundItem;
    }
    return nullptr;
}

void UAgoraItemLibrary::GenerateInventoryDebugString(AActor* InventoryOwner, FString& OutDebugString)
{
    if (!InventoryOwner) { return; }

    UAgoraInventoryComponent* InventoryComponent = InventoryOwner->FindComponentByClass<UAgoraInventoryComponent>();
    if (InventoryComponent)
    {
        InventoryComponent->GenerateDebugString(OutDebugString);
        return;
    }

    OutDebugString = FString::Printf(TEXT("No inventory component for %s"), *GetNameSafe(InventoryOwner));
}

bool UAgoraItemLibrary::CalculateItemCostFor(AActor* Actor, UAgoraItem* Item, float& OutCost)
{
    UAgoraInventoryComponent* InventoryComponent = GetInventoryComponent(Actor);
    if (InventoryComponent)
    {
        OutCost = InventoryComponent->CalculateItemCost(Item);
        return true;
    }
    return false;
}

bool UAgoraItemLibrary::CanPurchaseItem(AActor* Actor, UAgoraItem* Item)
{
    UAgoraInventoryComponent* InventoryComponent = GetInventoryComponent(Actor);
    if (InventoryComponent)
    {
        return InventoryComponent->CanPurchaseItem(Item);
    }

    return false;
}

bool UAgoraItemLibrary::CanSellAtInventorySlot(AActor* Actor, int32 SlotID)
{
    UAgoraInventoryComponent* InventoryComponent = GetInventoryComponent(Actor);
    if (InventoryComponent)
    {
        return InventoryComponent->CanSellAtInventorySlot(SlotID);
    }
    return false;
}

void UAgoraItemLibrary::GetBuildsIntoItemsFor(UAgoraItem* Item, TArray<UAgoraItem*>& OutBuildsIntoItems)
{
    if (!Item)
    {
        return;
    }

    OutBuildsIntoItems = Item->BuildsInto;
}

