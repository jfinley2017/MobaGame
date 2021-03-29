// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicAttacker.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "MeleeBasicAttacker.generated.h"

class USoundCue;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMeleeBasicAttacker : public UBasicAttacker
{
	GENERATED_BODY()
};

/**
 * 
 */
class AGORA_API IMeleeBasicAttacker
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	//Returns the result of executing a melee hit. This is usually just a call to UAgoraBlueprintFunctionLibrary::GenerateMeleeHitResult_Cleave().
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "MeleeAttacker")
    FMeleeHitResult GenerateMeleeHitResult();

	//Returns the sound played on a melee attack.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "MeleeAttacker")
	USoundCue* GetMeleeAttackSound();
	
};
