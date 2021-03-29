// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AgoraTypes.h"
#include "AgoraUIBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraUIBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    UFUNCTION(BlueprintPure, Category = "UIGlobals")
    static FString GetIdentifyingStringFor(AActor* Actor);

	// Turns something like manaCost to Mana Cost
	UFUNCTION(BlueprintPure, Category = "UIGlobals", meta = (AdvancedDisplay = 1))
	static FText GetAbilityStatDisplayText(FName AbilityStatName);
	
	// Get the display text for a given input slot
	UFUNCTION(BlueprintPure, Category = "UIGlobals", meta = (AdvancedDisplay = 1))
	static FText GetAbilityInputDisplayText(EAbilityInput AbilitySlot);

    UFUNCTION(BlueprintPure, Category = "UIGlobals")
    static void GetEnemyColor(FLinearColor& OutEnemyColor);

    UFUNCTION(BlueprintPure, Category = "UIGlobals")
    static void GetFriendlyColor(FLinearColor& OutFriendlyColor);

    UFUNCTION(BlueprintPure, Category = "UIGlobals")
    static void GetBrokenColor(FLinearColor& OutBrokenColor);

    UFUNCTION(BlueprintPure, Category = "UIGlobals")
    static void MakeGradient(const FLinearColor& BaseColor, FLinearColor& OutGradientHigh, FLinearColor& OutGradientLow, float GradientHighMultiplier, float GradientLowMultiplier);
};
