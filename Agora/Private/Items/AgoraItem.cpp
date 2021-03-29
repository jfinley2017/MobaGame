// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraItem.h"
#include "AgoraItemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AgoraInventoryComponent.h"


FString UAgoraItem::GetIdentifierString() const
{
    return GetPrimaryAssetId().ToString();
}

FString UAgoraItem::GetItemName() const
{
    return GetFName().ToString();
}

float UAgoraItem::GetTotalItemCost() const
{

    // If we've already calculated the cost for this item, return that
    if (HasCachedTotalItemCost)
    {
        return CachedTotalItemCost;
    }

    // Iterate through this item's children and their children's children.
    float ReturnedCost = Price;
    for (UAgoraItem* RequiredItem : RequiredItems)
    {
        ReturnedCost += RequiredItem->GetTotalItemCost();
    }

    // const cast to cache the calculated value, maintaining const on this method (we are not modifying
    // anything that is used outside the scope of this function)
    const_cast<UAgoraItem*>(this)->CachedTotalItemCost = ReturnedCost;
    const_cast<UAgoraItem*>(this)->HasCachedTotalItemCost = true;

    return ReturnedCost;
}

FPrimaryAssetId UAgoraItem::GetPrimaryAssetId() const
{
    // This is a DataAsset and not a blueprint so we can just use the raw FName
    // For blueprints you need to handle stripping the _C suffix
    return FPrimaryAssetId(UAgoraItemLibrary::AgoraItemAssetType, GetFName());
}

bool UAgoraItem::CanPurchase(UAgoraInventoryComponent* InventoryComponent)
{
    // refactored because CanAfford now takes into account items that exist in the passed in inventorycomponent
    return CanAfford(InventoryComponent);
}

float UAgoraItem::GetItemCostFor(UAgoraInventoryComponent* InventoryComponent)
{
    float ReturnedCost = Price;
    for (UAgoraItem* Item : RequiredItems)
    {
        if (InventoryComponent->GetItemCount(Item) > 0)
        {
            continue;
        }
        ReturnedCost += Item->GetItemCostFor(InventoryComponent);
    }
    return ReturnedCost;
}

bool UAgoraItem::CanAfford(UAgoraInventoryComponent* InventoryComponent)
{
    bool bFoundAttribute = false;
    float GoldAmount = UAbilitySystemBlueprintLibrary::GetFloatAttribute(InventoryComponent->GetOwner() 
                                                                         ,UAgoraAttributeSetEconomyBase::GetGoldAttribute(), bFoundAttribute);
    return bFoundAttribute && (GoldAmount >= GetItemCostFor(InventoryComponent));
}
