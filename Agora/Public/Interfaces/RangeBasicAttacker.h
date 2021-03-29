// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicAttacker.h"
#include "AgoraProjectileBase.h"
#include "RangeBasicAttacker.generated.h"



// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URangeBasicAttacker : public UBasicAttacker
{
	GENERATED_BODY()
};

/**
 * 
 */
class AGORA_API IRangeBasicAttacker
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	//Returns the location that a ranged unit's projectile should originate from. Will usually be some socket location, but it's obviously implementation-dependent.
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "RangeAttacker")
    FVector GetFiringLocation();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "RangeAttacker")
    FProjectileParams GetBasicAttackParams();


};
