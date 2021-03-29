// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "AgoraGameplayAbility.h"
#include "AgoraItem.generated.h"

class UAgoraInventoryComponent;

UENUM(BlueprintType)
enum class EAgoraItemAttributeModType : uint8
{
    Add,
    Multiply,
};

USTRUCT(BlueprintType)
struct FAgoraItemAttributeModifier
{
    GENERATED_BODY()
    
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    FGameplayAttribute Attribute;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    EAgoraItemAttributeModType AttributeModType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    float Magnitude;
};

USTRUCT(BlueprintType)
struct FAgoraUniqueItemAttributeModifier
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    FGameplayTag UniqueIdentifier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    FAgoraItemAttributeModifier AttributeModifier;

};
    
USTRUCT(BlueprintType)
struct FAgoraUniqueItemEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    FGameplayTag UniqueIdentifier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    TSubclassOf<UGameplayEffect> UniqueEffect;

};

/**
 * 
 */
UCLASS(Blueprintable)
class AGORA_API UAgoraItem : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

    UAgoraItem() {}

    // UPrimaryDataAsset
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
    // ~UPrimaryDataAsset

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    FText ItemDescription;

    /** Price for this item. If I have a child that costs 300g and I am 400g, our total is 700g. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    int32 Price;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    TArray<UAgoraItem*> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    TArray<FAgoraItemAttributeModifier> AttributeModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    TArray<FAgoraUniqueItemAttributeModifier> UniqueAttributeModifiers;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    TArray<TSubclassOf<UGameplayEffect>> ItemEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    TArray<FAgoraUniqueItemEffect> UniqueItemEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AgoraItem")
    TSubclassOf<UAgoraGameplayAbility> ActiveAbility;
   
    UFUNCTION(BlueprintCallable, Category = "AgoraItem")
    FString GetIdentifierString() const;

    UFUNCTION(BlueprintPure, Category = "AgoraItem")
    FString GetItemName() const;

    UFUNCTION(BlueprintPure, Category = "AgoraItem")
    float GetTotalItemCost() const;

    /**
     * Not exposed, generated at runtime to avoid designer overhead.
     */
    UPROPERTY(BlueprintReadOnly, Category = "AgoraItem")
    TArray<UAgoraItem*> BuildsInto;

    /**
     * Returns true if the inventory component (and owning actor) can buy this item.
     */
    bool CanPurchase(UAgoraInventoryComponent* InventoryComponent);

    /**
     * Returns the item cost for the inventory component. Takes in account parts of the item that are already owned.
     */
    float GetItemCostFor(UAgoraInventoryComponent* InventoryComponent);

protected:

    /*
    * Returns true if the inventory component (and owning actor) can afford this item
    */
    bool CanAfford(UAgoraInventoryComponent* InventoryComponent);

    // Cache variables to avoid repeat recursive calculations.
    float CachedTotalItemCost = 0.0f;
    bool HasCachedTotalItemCost = false;

};
