// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "AgoraItemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Agora.h"
#include "AgoraAttributeSetEconomyBase.h"
#include "UnrealMathUtility.h"
#include "AgoraDataSingleton.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagContainer.h"
#include "AgoraGameplayTags.h"
#include "Components/InputComponent.h"
#include "GameFramework/Character.h"
#include "AgoraHeroBase.h"

// Sets default values for this component's properties
UAgoraInventoryComponent::UAgoraInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
    bReplicates = true;
	// ...
}

// Called when the game starts
void UAgoraInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

    // @TODO move this elsewhere, initial gold
    UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
    UGameplayEffect* ItemStaticModifierGE = NewObject<UGameplayEffect>();
    ItemStaticModifierGE->DurationPolicy = EGameplayEffectDurationType::Instant;
    FGameplayModifierInfo GameplayModInfo;
    GameplayModInfo.Attribute = UAgoraAttributeSetEconomyBase::GetGoldAttribute();
    GameplayModInfo.ModifierMagnitude = FScalableFloat(500.0f);
    GameplayModInfo.ModifierOp = EGameplayModOp::Additive;
    ItemStaticModifierGE->Modifiers.Add(GameplayModInfo);
    OwnerASC->ApplyGameplayEffectToSelf(ItemStaticModifierGE, 1.0f, OwnerASC->MakeEffectContext());
    
    AAgoraHeroBase* OwnerAsHero = Cast<AAgoraHeroBase>(GetOwner());
    if (OwnerAsHero)
    {
        OwnerAsHero->OnInputComponentReady.AddDynamic(this, &UAgoraInventoryComponent::SetupInventoryInput);
    }

    if (GetOwner()->HasAuthority())
    {
        for (int i = 0; i < NumInventorySlots; i++)
        {
            FAgoraInventorySlot NewSlot;
            NewSlot.SlotID = i;
            Inventory.Add(NewSlot);
        }
    }
}

// Called every frame
void UAgoraInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAgoraInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAgoraInventoryComponent, Inventory);

}

int32 UAgoraInventoryComponent::FindSlotFromItem(const UAgoraItem* Item, FAgoraInventorySlot& OutItemSlot)
{
    for (int32 i= 0; i < Inventory.Num(); i++)
    {
        if (Inventory[i].SlottedItem.Item == Item)
        {
            OutItemSlot = Inventory[i];
            return i;
        }
    }
    return -1;
}

bool UAgoraInventoryComponent::HasRoomForItem(const UAgoraItem* Item)
{
    if (!Item)
    {
        return false;
    }

    // If we have a child item, we can replace it when we are purchased
    for (const UAgoraItem* RequiredItem : Item->RequiredItems)
    {
        if (GetItemCount(RequiredItem))
        {
            return true;
        }
    }

    int32 ThrowAwayEmptySlotIdx = -1;
    return FindEmptySlot(ThrowAwayEmptySlotIdx);
}

bool UAgoraInventoryComponent::FindEmptySlot(int32& OutEmptySlot)
{
    for (int32 i = 0; i < Inventory.Num(); i++)
    {
        if (Inventory[i].IsEmpty())
        {
            OutEmptySlot = i;
            return true;
        }
    }
    return false;
}


bool UAgoraInventoryComponent::TryBuyItem(UAgoraItem* Item)
{
    if (UAgoraItemLibrary::CanPurchaseItem(GetOwner(),Item))
    {
        //#SOUNDCUE
        UGameplayStatics::PlaySound2D(GetWorld(), UAgoraBlueprintFunctionLibrary::GetGlobals()->BuyItemSuccess, .5f, 1.f, 0.f, nullptr, GetOwner());
        Server_TryBuyItem(Item);
        return true;
    }

    return false;
}

bool UAgoraInventoryComponent::TrySellItem(int32 SlotToSellAt)
{
    if (UAgoraItemLibrary::CanSellAtInventorySlot(GetOwner(), SlotToSellAt))
    {
        //#SOUNDCUE
        UGameplayStatics::PlaySound2D(GetWorld(), UAgoraBlueprintFunctionLibrary::GetGlobals()->SellItemSuccess, .5f, 1.f, 0.f, nullptr, GetOwner());
        Server_TrySellItem(SlotToSellAt);
        return true;
    }
    return false;
}

void UAgoraInventoryComponent::EquipItem(UAgoraItem* Item, int32 Count)
{
    if (!GetOwner()->HasAuthority()) { return; }

    int32 SlotToPlaceAt = -1;
    if (FindEmptySlot(SlotToPlaceAt))
    {
        EquipItemAtSlot(Item, Count, SlotToPlaceAt);
    }
}

void UAgoraInventoryComponent::RemoveItem(UAgoraItem* Item, int32 Count)
{
    if (!GetOwner()->HasAuthority()) { return; }

    FAgoraInventorySlot Slot;
    int32 ItemSlot = FindSlotFromItem(Item, Slot);
    if (ItemSlot != -1)
    {
        RemoveItemAtSlot(1, ItemSlot);
    }
}

void UAgoraInventoryComponent::EquipItemAtSlot(UAgoraItem* Item, int32 Count, int32 Slot)
{
    if (!GetOwner()->HasAuthority()) { return; }

    if (!Item) { TRACE(AgoraItem, Error, "Item was NULL when attempting to equip at slot %d", Slot); return; }

    UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
    if (!OwnerASC) { return; }

    FAgoraActiveItem NewItem;
    NewItem.Item = Item;
    
    {
        // Apply item static modifiers through an anonymous GE
        UGameplayEffect* ItemStaticModifierGE = NewObject<UGameplayEffect>();
        ItemStaticModifierGE->DurationPolicy = EGameplayEffectDurationType::Instant;
        for (FAgoraItemAttributeModifier& ItemAttributeMod : Item->AttributeModifiers)
        {
            FGameplayModifierInfo GameplayModInfo;
            MakeGameplayModifierInfoFromItemMod(ItemAttributeMod, GameplayModInfo);
            ItemStaticModifierGE->Modifiers.Add(GameplayModInfo);
        }
        OwnerASC->ApplyGameplayEffectToSelf(ItemStaticModifierGE, 1.0f, OwnerASC->MakeEffectContext());
    }

    for (FAgoraUniqueItemAttributeModifier& UniqueAttributeModifier : Item->UniqueAttributeModifiers)
    {
        if (!UniqueProviders.Contains(UniqueAttributeModifier.UniqueIdentifier))
        {
            UGameplayEffect* ItemStaticModifierGE = NewObject<UGameplayEffect>();
            ItemStaticModifierGE->DurationPolicy = EGameplayEffectDurationType::Instant;
            for (FAgoraUniqueItemAttributeModifier& ItemAttributeMod : Item->UniqueAttributeModifiers)
            {
                FGameplayModifierInfo GameplayModInfo;
                MakeGameplayModifierInfoFromItemMod(ItemAttributeMod.AttributeModifier, GameplayModInfo);
                ItemStaticModifierGE->Modifiers.Add(GameplayModInfo);
            }
            OwnerASC->ApplyGameplayEffectToSelf(ItemStaticModifierGE, 1.0f, OwnerASC->MakeEffectContext());

            UniqueProviders.Add(UniqueAttributeModifier.UniqueIdentifier, Inventory[Slot].SlottedItem);
        }
    }

    for (TSubclassOf<UGameplayEffect> ItemEffect : Item->ItemEffects)
    {
        FGameplayEffectSpecHandle ItemEffectSpecHande = OwnerASC->MakeOutgoingSpec(ItemEffect, 1.0f, OwnerASC->MakeEffectContext());
        FActiveGameplayEffectHandle ActiveItemEffectHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*ItemEffectSpecHande.Data);
        NewItem.ActiveEffects.Add(ActiveItemEffectHandle);
    }

    for (FAgoraUniqueItemEffect& UniqueItemEffect : Item->UniqueItemEffects)
    {
        if (!UniqueProviders.Contains(UniqueItemEffect.UniqueIdentifier))
        {
            FGameplayEffectSpecHandle ItemEffectSpecHande = OwnerASC->MakeOutgoingSpec(UniqueItemEffect.UniqueEffect, 1.0f, OwnerASC->MakeEffectContext());
            FActiveGameplayEffectHandle ActiveItemEffectHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*ItemEffectSpecHande.Data);
            NewItem.ActiveUniqueEffects.Add(FAgoraActiveUniqueEffect(UniqueItemEffect.UniqueIdentifier, ActiveItemEffectHandle));
            UniqueProviders.Add(UniqueItemEffect.UniqueIdentifier,NewItem);
        }
    }

    if (Item->ActiveAbility)
    {
        FGameplayAbilitySpec ItemAbilitySpec = FGameplayAbilitySpec(Item->ActiveAbility, 1.0f, -1, nullptr);
        NewItem.ActiveAbility = OwnerASC->GiveAbility(ItemAbilitySpec);
        AbilityProvider.Add(Item->ActiveAbility, NewItem);
    }

    Inventory[Slot].SlottedItem = NewItem;
    OnItemSlotUpdated.Broadcast(Inventory[Slot]);

    TRACE(AgoraItem, Log, "Item %s added to %s", *GetNameSafe(Item), *GetNameSafe(GetOwner()));
}

void UAgoraInventoryComponent::RemoveItemAtSlot(int32 Count, int32 Slot)
{
    if (!GetOwner()->HasAuthority()) { return; }

    UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
    if (!OwnerASC) { return; }

    FAgoraActiveItem ActiveItem = Inventory[Slot].SlottedItem;
    const UAgoraItem* Item = ActiveItem.Item;
    
    // Undo Static stats
    {
        UGameplayEffect* ItemStaticModifierGE = NewObject<UGameplayEffect>();
        ItemStaticModifierGE->DurationPolicy = EGameplayEffectDurationType::Instant;

        for (const FAgoraItemAttributeModifier& ItemAttributeMod : Item->AttributeModifiers)
        {
            FGameplayModifierInfo GameplayModInfo;
            MakeNegationGameplayModifierInfoFromItemMod(ItemAttributeMod, GameplayModInfo);
            ItemStaticModifierGE->Modifiers.Add(GameplayModInfo);
        }
        OwnerASC->ApplyGameplayEffectToSelf(ItemStaticModifierGE, 1.0f, OwnerASC->MakeEffectContext());
    }
    
    // Undo any unique stats we're applying
    for (const FAgoraUniqueItemAttributeModifier& UniqueAttributeModifier : Item->UniqueAttributeModifiers)
    {
        if (IsProviderOfUniqueEffect(ActiveItem,UniqueAttributeModifier.UniqueIdentifier))
        {
            UGameplayEffect* ItemStaticModifierGE = NewObject<UGameplayEffect>();
            ItemStaticModifierGE->DurationPolicy = EGameplayEffectDurationType::Instant;
            for (const FAgoraUniqueItemAttributeModifier& ItemAttributeMod : Item->UniqueAttributeModifiers)
            {
                FGameplayModifierInfo GameplayModInfo;
                MakeNegationGameplayModifierInfoFromItemMod(ItemAttributeMod.AttributeModifier, GameplayModInfo);
                ItemStaticModifierGE->Modifiers.Add(GameplayModInfo);
            }
            OwnerASC->ApplyGameplayEffectToSelf(ItemStaticModifierGE, 1.0f, OwnerASC->MakeEffectContext());
            UniqueProviders.Remove(UniqueAttributeModifier.UniqueIdentifier);
        }
    }

    // Remove any active effects
    for (FActiveGameplayEffectHandle& ActiveItemEffect : ActiveItem.ActiveEffects)
    {
        OwnerASC->RemoveActiveGameplayEffect(ActiveItemEffect);
    }

    // Remove any active unique effects
    for (FAgoraActiveUniqueEffect& ActiveUniqueItemEffects : ActiveItem.ActiveUniqueEffects)
    {
        OwnerASC->RemoveActiveGameplayEffect(ActiveUniqueItemEffects.ActiveEffectHandle);
        UniqueProviders.Remove(ActiveUniqueItemEffects.EffectTag);
    }

    // Remove any abilities
    if (ActiveItem.ActiveAbility.IsValid())
    {
        OwnerASC->CancelAbilityHandle(ActiveItem.ActiveAbility);
        OwnerASC->SetRemoveAbilityOnEnd(ActiveItem.ActiveAbility);
    }


    FAgoraActiveItem EmptyItem;
    Inventory[Slot].SlottedItem = EmptyItem;
    OnItemSlotUpdated.Broadcast(Inventory[Slot]);

    RegenerateInventoryEffectsPostItemRemoval();

    TRACE(AgoraItem, Log, "Item %s removed from %s", *GetNameSafe(Item), *GetNameSafe(GetOwner()));
}

bool UAgoraInventoryComponent::GetInventorySlotAt(int32 SlotIndex, FAgoraInventorySlot& OutSlot)
{
    if (Inventory.IsValidIndex(SlotIndex))
    {
        OutSlot = Inventory[SlotIndex];
        return true;
    }
    return false;
}

void UAgoraInventoryComponent::GetAllInventorySlots(TArray<FAgoraInventorySlot>& Slots)
{
    Slots = Inventory;
}

float UAgoraInventoryComponent::CalculateItemCost(UAgoraItem* Item)
{
    return Item->GetItemCostFor(this);
}

float UAgoraInventoryComponent::GetItemSellPrice(const UAgoraItem* Item)
{
    return Item->GetTotalItemCost() * .75;
}

void UAgoraInventoryComponent::MakeGameplayModifierInfoFromItemMod(const FAgoraItemAttributeModifier& ItemMod, FGameplayModifierInfo& OutGameplayModInfo)
{
    OutGameplayModInfo.ModifierOp = ItemMod.AttributeModType == EAgoraItemAttributeModType::Add
        ? EGameplayModOp::Additive : EGameplayModOp::Multiplicitive;
    OutGameplayModInfo.Attribute = ItemMod.Attribute;
    OutGameplayModInfo.ModifierMagnitude = FScalableFloat(ItemMod.Magnitude);

}

void UAgoraInventoryComponent::MakeNegationGameplayModifierInfoFromItemMod(const FAgoraItemAttributeModifier& ItemMod, FGameplayModifierInfo& OutGameplayModInfo)
{
    OutGameplayModInfo.Attribute = ItemMod.Attribute;

    if (ItemMod.AttributeModType == EAgoraItemAttributeModType::Add)
    {
        OutGameplayModInfo.ModifierOp = EGameplayModOp::Additive;
        OutGameplayModInfo.ModifierMagnitude = FScalableFloat(ItemMod.Magnitude * -1);
    }
    else
    {
        OutGameplayModInfo.ModifierOp = EGameplayModOp::Multiplicitive;
        OutGameplayModInfo.ModifierMagnitude = FScalableFloat(1 / ItemMod.Magnitude);
    }
}

bool UAgoraInventoryComponent::IsProviderOfUniqueEffect(const FAgoraActiveItem& Item, FGameplayTag UniqueEffectIdentifier)
{
    return UniqueProviders.Contains(UniqueEffectIdentifier) && UniqueProviders[UniqueEffectIdentifier].UniqueItemID == Item.UniqueItemID;
}

void UAgoraInventoryComponent::RegenerateInventoryEffectsPostItemRemoval()
{

    UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

    for (FAgoraInventorySlot& InventorySlot : Inventory)
    {
        if (InventorySlot.IsEmpty()) { continue; }

        FAgoraActiveItem& ActiveItem = InventorySlot.SlottedItem;
        const UAgoraItem* ItemDef = ActiveItem.Item;
        
        for (const FAgoraUniqueItemAttributeModifier& UniqueAttributeModifier : ItemDef->UniqueAttributeModifiers)
        {
            if (!UniqueProviders.Contains(UniqueAttributeModifier.UniqueIdentifier))
            {
                UGameplayEffect* ItemStaticModifierGE = NewObject<UGameplayEffect>();
                ItemStaticModifierGE->DurationPolicy = EGameplayEffectDurationType::Instant;
                for (const FAgoraUniqueItemAttributeModifier& ItemAttributeMod : ItemDef->UniqueAttributeModifiers)
                {
                    FGameplayModifierInfo GameplayModInfo;
                    MakeGameplayModifierInfoFromItemMod(ItemAttributeMod.AttributeModifier, GameplayModInfo);
                    ItemStaticModifierGE->Modifiers.Add(GameplayModInfo);
                }
                OwnerASC->ApplyGameplayEffectToSelf(ItemStaticModifierGE, 1.0f, OwnerASC->MakeEffectContext());
                UniqueProviders.Add(UniqueAttributeModifier.UniqueIdentifier, ActiveItem);
            }
        }

        for (const FAgoraUniqueItemEffect& UniqueItemEffect : ItemDef->UniqueItemEffects)
        {
            if (!UniqueProviders.Contains(UniqueItemEffect.UniqueIdentifier))
            {
                FGameplayEffectSpecHandle ItemEffectSpecHande = OwnerASC->MakeOutgoingSpec(UniqueItemEffect.UniqueEffect, 1.0f, OwnerASC->MakeEffectContext());
                FActiveGameplayEffectHandle ActiveItemEffectHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*ItemEffectSpecHande.Data);
                ActiveItem.ActiveUniqueEffects.Add(FAgoraActiveUniqueEffect(UniqueItemEffect.UniqueIdentifier, ActiveItemEffectHandle));
                UniqueProviders.Add(UniqueItemEffect.UniqueIdentifier, ActiveItem);
            }
        }
    }
}

bool UAgoraInventoryComponent::CanSellAtInventorySlot(int32 SlotID)
{
    FAgoraInventorySlot& InventorySlot = Inventory[SlotID];
    return !InventorySlot.IsEmpty();
}

float UAgoraInventoryComponent::GetItemPriceAtInventorySlot(int32 SlotID)
{
    FAgoraInventorySlot& InventorySlot = Inventory[SlotID];
    return InventorySlot.IsEmpty() ? 0.0f : GetItemSellPrice(Inventory[SlotID].SlottedItem.Item);
}

int32 UAgoraInventoryComponent::GetItemCount(const UAgoraItem* Item)
{
    int32 ReturnedCount = 0;
    for (FAgoraInventorySlot& ItemSlot : Inventory)
    {
        if (ItemSlot.SlottedItem.Item == Item)
        {
            ReturnedCount++;
        }
    }
    return ReturnedCount;
}

void UAgoraInventoryComponent::TryUseInventorySlot(int32 Idx)
{
    if (!Inventory.IsValidIndex(Idx))
    {
        TRACE(AgoraItem, Error, "Tried to use item at %d, but %d was not a valid inventory index.", Idx);
        return;
    }

    if (Inventory[Idx].IsEmpty() || !Inventory[Idx].SlottedItem.ActiveAbility.IsValid())
    {
        return;
    }

    UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
    if (OwnerASC)
    {
        OwnerASC->TryActivateAbility(Inventory[Idx].SlottedItem.ActiveAbility);
    }

}

void UAgoraInventoryComponent::SetupInventoryInput(UInputComponent* InputComponent)
{
    InputComponent->BindAction<FUseInventorySlot>("UseInventorySlotOne", IE_Pressed, this, &UAgoraInventoryComponent::TryUseInventorySlot, 0);
    InputComponent->BindAction<FUseInventorySlot>("UseInventorySlotTwo", IE_Pressed, this, &UAgoraInventoryComponent::TryUseInventorySlot, 1);
    InputComponent->BindAction<FUseInventorySlot>("UseInventorySlotThree", IE_Pressed, this, &UAgoraInventoryComponent::TryUseInventorySlot, 2);
    InputComponent->BindAction<FUseInventorySlot>("UseInventorySlotFour", IE_Pressed, this, &UAgoraInventoryComponent::TryUseInventorySlot, 3);
    InputComponent->BindAction<FUseInventorySlot>("UseInventorySlotFive", IE_Pressed, this, &UAgoraInventoryComponent::TryUseInventorySlot, 4);
    InputComponent->BindAction<FUseInventorySlot>("UseInventorySlotSix", IE_Pressed, this, &UAgoraInventoryComponent::TryUseInventorySlot, 5);
}

bool UAgoraInventoryComponent::CanPurchaseItem(UAgoraItem* Item)
{
    FGameplayTagContainer BuyTagContainer;
    BuyTagContainer.AddTag(UAgoraGameplayTags::Dead());
    BuyTagContainer.AddTag(FGameplayTag::RequestGameplayTag("Location.Shop"));
    bool OwnerHasTags = UAgoraBlueprintFunctionLibrary::HasAnyMatchingGameplaytags(GetOwner(), BuyTagContainer);

    return HasRoomForItem(Item) && Item->CanPurchase(this) && OwnerHasTags;
}

void UAgoraInventoryComponent::Server_TryBuyItem_Implementation(UAgoraItem* Item)
{
    if (UAgoraItemLibrary::CanPurchaseItem(GetOwner(),Item))
    {
        // Determine how much we're paying for this item, before we remove the children
        // (and making it more expensive)
        float ItemCost = Item->GetItemCostFor(this);

        for (UAgoraItem* RequiredItem : Item->RequiredItems)
        {
            RemoveItem(RequiredItem,1);
        }
        EquipItem(Item, 1.0);

        // Apply cost
        UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
        if (!OwnerASC) { TRACE(AgoraItem, Error, "Tried to charge %s for %s, but could not find AbilitySystemComponent"); return; }

        UGameplayEffect* ItemStaticModifierGE = NewObject<UGameplayEffect>();
        ItemStaticModifierGE->DurationPolicy = EGameplayEffectDurationType::Instant;

        FGameplayModifierInfo GameplayModInfo;
        GameplayModInfo.Attribute = UAgoraAttributeSetEconomyBase::GetGoldAttribute();
        GameplayModInfo.ModifierMagnitude = FScalableFloat(-1 * ItemCost);
        GameplayModInfo.ModifierOp = EGameplayModOp::Additive;

        ItemStaticModifierGE->Modifiers.Add(GameplayModInfo);

        OwnerASC->ApplyGameplayEffectToSelf(ItemStaticModifierGE, 1.0f, OwnerASC->MakeEffectContext());

        TRACE(AgoraItem, Log, "%s purchased item %s for %d gold.", *GetNameSafe(GetOwner()), *Item->ItemName.ToString(), Item->Price);
    }
}

bool UAgoraInventoryComponent::Server_TryBuyItem_Validate(UAgoraItem* Item)
{
    return true;
}

void UAgoraInventoryComponent::Server_TrySellItem_Implementation(int32 ItemSlot)
{
    if (UAgoraItemLibrary::CanSellAtInventorySlot(GetOwner(),ItemSlot))
    {
        
        UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
        if (!OwnerASC) { /** yikes */ return; }

        const UAgoraItem* ItemAtSlot = Inventory[ItemSlot].SlottedItem.Item;
        int32 ItemSellPrice = FMath::FloorToInt(GetItemSellPrice(ItemAtSlot));

        UGameplayEffect* ItemStaticModifierGE = NewObject<UGameplayEffect>();
        ItemStaticModifierGE->DurationPolicy = EGameplayEffectDurationType::Instant;

        FGameplayModifierInfo GameplayModInfo;
        GameplayModInfo.Attribute = UAgoraAttributeSetEconomyBase::GetGoldAttribute();
        GameplayModInfo.ModifierMagnitude = FScalableFloat(ItemSellPrice);
        GameplayModInfo.ModifierOp = EGameplayModOp::Additive;

        ItemStaticModifierGE->Modifiers.Add(GameplayModInfo);

        OwnerASC->ApplyGameplayEffectToSelf(ItemStaticModifierGE, 1.0f, OwnerASC->MakeEffectContext());

        RemoveItemAtSlot(1,ItemSlot);

        TRACE(AgoraItem, Log, "%s sold item %s for %d gold.", *GetNameSafe(GetOwner()), *ItemAtSlot->ItemName.ToString(), ItemSellPrice);
    }
}

bool UAgoraInventoryComponent::Server_TrySellItem_Validate(int32 ItemSlot)
{
    return true;
}

void UAgoraInventoryComponent::OnRep_Inventory()
{
    for (FAgoraInventorySlot& Slot : Inventory)
    {
        OnItemSlotUpdated.Broadcast(Slot);
    }
}

//////////////////////////////////////////////////////////////////////////
// ItemStructs
//////////////////////////////////////////////////////////////////////////

UTexture2D* FAgoraInventorySlot::GetItemIcon()
{
    return IsEmpty() ? nullptr : SlottedItem.Item->Icon;
}


//////////////////////////////////////////////////////////////////////////
// Debug
//////////////////////////////////////////////////////////////////////////

void UAgoraInventoryComponent::GenerateDebugString(FString& OutDebugString)
{
    OutDebugString = "";

    OutDebugString += FString::Printf(TEXT("Inventory for %s: \n"), *GetNameSafe(GetOwner()));
    for (int i = 0; i < Inventory.Num(); i++)
    {
        OutDebugString += FString::Printf(TEXT("%s,"), Inventory[i].IsEmpty() ? TEXT("Empty") : *Inventory[i].SlottedItem.Item->GetItemName());
    }
}


