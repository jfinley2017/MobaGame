// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BasicAttacker.generated.h"

class UBasicAttackComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBasicAttacker : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AGORA_API IBasicAttacker
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
    // Perhaps a basic attack unit would like to swap basic attacks, by overriding this function we can essentially
    // choose which component is our current component
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "BasicAttacker")
    UBasicAttackComponent* GetBasicAttackComponent();

    // Some units may have different ways of retrieving their basic attack cooldowns
    // an argument could be made that we should just attempt to grab some defined attribute set and tell it to calculate the cooldown for us
    // but im including this function to be overridden here for convenience (and because such an attribute set doesnnt exist)
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "BasicAttacker")
    float GetBasicAttackCooldown();


   
};
