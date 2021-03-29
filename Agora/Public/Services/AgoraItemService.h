// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "AgoraItemService.generated.h"

class UAgoraItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemsLoadedSignature);

/**
 * 
 */
UCLASS()
class AGORA_API AAgoraItemService : public AInfo
{
	GENERATED_BODY()
	
public:

    AAgoraItemService();

    UPROPERTY(BlueprintAssignable, Category = "AgoraItem")
    FOnItemsLoadedSignature OnItemsLoaded;

    // AInfo
    virtual void PreInitializeComponents() override;
    // ~AInfo

    /** Retrieves all loaded items, sorted by price */
    UFUNCTION(BlueprintPure, Category = "AgoraItem")
    void GetItems(TArray<UAgoraItem*>& OutItems);

    /** Returns an item designated by ItemName, where ItemName is the asset name of the item*/
    UFUNCTION(BlueprintPure, Category = "AgoraItem")
    UAgoraItem* GetItemFromName(const FString& ItemName);

    /** Returns an item from a FPrimaryAssetID */
    UFUNCTION(BlueprintPure, Category = "AgoraItem")
    UAgoraItem* GetItemFromPrimaryID(FPrimaryAssetId AssetID);


protected:

    // AInfo
    virtual void BeginPlay() override;
    // ~AInfo

    /** Map containing all items loaded. */
    UPROPERTY()
    TMap<FPrimaryAssetId, UAgoraItem*> LoadedItems;

    UPROPERTY(ReplicatedUsing=OnRep_SortedItems)
    TArray<UAgoraItem*> SortedItems;

    UFUNCTION()
    void Internal_NotifyItemsLoaded();

    UFUNCTION()
    void OnRep_SortedItems();

};
