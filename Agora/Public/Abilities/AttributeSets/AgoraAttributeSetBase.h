// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h" // Attribute macros require this
#include "AttributeSet.h"
#include "AgoraAttributeSetBase.generated.h"


/*********************************************************************************************************/
/*                                             Base Attribute Set                                        */
/* - The base set which other sets derive from                                                           */
/* - 'Actual' Attribute sets should inherit from here, this set just contains utility functions which    */
/*    the other 'actual' sets utilize.                                                                   */
/*********************************************************************************************************/

// Updates attributes that are concerned about the current and maximum values (eg. Health, Mana, etc)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDoubleAttributeUpdateSignature, float, CurrentValue, float, MaxValue);

// Provides updates for attributes that are concerned about current values, (eg. AttackSpeed, CooldownReduction, etc)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSingleAttributeUpdateSignature, float, CurrentAttributeData);

// Macros to make life easier, this macro combines the already given macros in AttributeSet.h
// to allow for simple getters/setters of attributes.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class AGORA_API UAgoraAttributeSetBase : public UAttributeSet
{
    GENERATED_BODY()

public:

    UAgoraAttributeSetBase();

    // These are ran exclusively on the server. Typically in order for a client to react to an attribute being changed, we use the OnRep_ functions
    // which are called when a variable marked as replicated is changed on the server
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;

    //////////////////////////////////////////////////////////////////////////
    // Attributes

   
protected:

	// Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes. (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before) 
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

    
};
