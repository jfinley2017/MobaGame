// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraItemService.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "AgoraItemLibrary.h"
#include "AgoraItem.h"

AAgoraItemService::AAgoraItemService()
{
    SetReplicates(true);
    bAlwaysRelevant = true;
}

void AAgoraItemService::BeginPlay()
{
    Super::BeginPlay();


}

void AAgoraItemService::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAgoraItemService, SortedItems);
}

void AAgoraItemService::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    if (HasAuthority())
    {
        UAssetManager* AssetManager = GEngine->AssetManager;
        TArray<FPrimaryAssetId> PrimaryAsset;
        AssetManager->GetPrimaryAssetIdList(UAgoraItemLibrary::AgoraItemAssetType, PrimaryAsset);
        FStreamableDelegate ItemsLoadedDelegate;
        ItemsLoadedDelegate.BindUFunction(this, "Internal_NotifyItemsLoaded");
        AssetManager->LoadPrimaryAssets(PrimaryAsset, TArray<FName>(), ItemsLoadedDelegate);
    }
    
}

void AAgoraItemService::GetItems(TArray<UAgoraItem*>& OutItems)
{
    OutItems = SortedItems;
}

UAgoraItem* AAgoraItemService::GetItemFromName(const FString& ItemName)
{
    return GetItemFromPrimaryID(FPrimaryAssetId(UAgoraItemLibrary::AgoraItemAssetType, FName(*ItemName)));
}

UAgoraItem* AAgoraItemService::GetItemFromPrimaryID(FPrimaryAssetId AssetID)
{
    UAssetManager* AssetManager = GEngine->AssetManager;
    return AssetManager->GetPrimaryAssetObject<UAgoraItem>(AssetID);
}

void AAgoraItemService::Internal_NotifyItemsLoaded()
{
    UAssetManager* AssetManager = GEngine->AssetManager;

    TArray<UObject*> Items;
    AssetManager->GetPrimaryAssetObjectList(UAgoraItemLibrary::AgoraItemAssetType, Items);
    for (UObject* Item : Items)
    {
        UAgoraItem* ItemAsAgoraItem = Cast<UAgoraItem>(Item);
        LoadedItems.Add(ItemAsAgoraItem->GetPrimaryAssetId(), ItemAsAgoraItem);
        SortedItems.Add(ItemAsAgoraItem);
        TRACE(AgoraItem, Log, "%s Loaded.", *ItemAsAgoraItem->GetIdentifierString());
    }

    // sort by total price
    int i, j;
    for (i = 0; i < SortedItems.Num(); i++)
    {
        for (j = 0; j < SortedItems.Num() - i - 1; j++)
        {
            if (SortedItems[j]->GetTotalItemCost() > SortedItems[j + 1]->GetTotalItemCost())
            {
                UAgoraItem* Temp = SortedItems[j];
                SortedItems[j] = SortedItems[j + 1];
                SortedItems[j + 1] = Temp;
            }
        }
    }

    for (UAgoraItem* Item : SortedItems)
    {
        for (UAgoraItem* RequiredItem : Item->RequiredItems)
        {
            RequiredItem->BuildsInto.AddUnique(Item);
        }
    }

    TRACE(AgoraItem, Log, "Items loaded.");
    OnItemsLoaded.Broadcast();
}

void AAgoraItemService::OnRep_SortedItems()
{
    OnItemsLoaded.Broadcast();
}
