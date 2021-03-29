// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayEffect.h"
#include "AgoraItemLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AgoraItem, Log, All);
DECLARE_STATS_GROUP(TEXT("AgoraItems"), STATGROUP_AgoraItems, STATCAT_Advanced);

class UTexture2D;
class AAgoraItemService;

/**
 * Provides a static interface for manipulating item related classes.
 */
UCLASS()
class AGORA_API UAgoraItemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    static const FPrimaryAssetType AgoraItemAssetType;

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraItemLibrary")
    static AAgoraItemService* GetItemService(UObject* WorldContextObject);

    /**
     * Grabs the inventory component from an actor
     */
    UFUNCTION(BlueprintPure, Category = "AgoraItemLibrary")
    static UAgoraInventoryComponent* GetInventoryComponent(AActor* Actor);

    /**
     * Returns all loaded items in the world.
     */
    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraItemLibrary")
    static void GetItems(UObject* WorldContextObject, TArray<UAgoraItem*>& Items);

    /**
     * Returns an item with the name @ItemName. ItemName is designated by the asset name, if the data table is named "Test", then ItemName should be "Test".
     */
    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraItemLibrary")
    static UAgoraItem* GetItemByName(UObject* WorldContextObject, const FString& ItemName);

    /**
     * Generates a debug string for the InventoryOwner, printing out the current contents of the associated inventory.
     */
    UFUNCTION(BlueprintPure, Category = "AgoraItemLibrary")
    static void GenerateInventoryDebugString(AActor* InventoryOwner, FString& OutDebugString);

    UFUNCTION(BlueprintPure, Category = "AgoraItemLibrary")
    static bool CalculateItemCostFor(AActor* Actor, UAgoraItem* Item, float& OutCost);

    UFUNCTION(BlueprintPure, Category = "AgoraItemLibrary")
    static bool CanPurchaseItem(AActor* Actor, UAgoraItem* Item);

    UFUNCTION(BlueprintPure, Category = "AgoraItemLibrary")
    static bool CanSellAtInventorySlot(AActor* Actor, int32 SlotID);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AgoraItemLibrary")
    static void GetBuildsIntoItemsFor(UAgoraItem* Item, TArray<UAgoraItem*>& OutBuildsIntoItems);
};
