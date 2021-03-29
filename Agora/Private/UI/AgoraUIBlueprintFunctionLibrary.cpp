// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "AgoraUIBlueprintFunctionLibrary.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraDataSingleton.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"

FString UAgoraUIBlueprintFunctionLibrary::GetIdentifyingStringFor(AActor* Actor)
{

#ifndef WITH_EDITOR
    APawn* ActorAsPawn = Cast<APawn>(Actor);
    if (ActorAsPawn)
    {
        if (ActorAsPawn->IsPlayerControlled())
        {
            return ActorAsPawn->GetPlayerState()->GetPlayerName();
        }
    }
#endif
    return GetNameSafe(Actor);
}

FText UAgoraUIBlueprintFunctionLibrary::GetAbilityStatDisplayText(FName AbilityStatName)
{
	FText* Result = UAgoraBlueprintFunctionLibrary::GetGlobals()->AbilityStatNameMap.Find(AbilityStatName);
	if (Result)
	{
		return *Result;
	}

	return FText();
}

FText UAgoraUIBlueprintFunctionLibrary::GetAbilityInputDisplayText(EAbilityInput AbilitySlot)
{
	FText* Result = UAgoraBlueprintFunctionLibrary::GetGlobals()->AbilityInputNameMap.Find(AbilitySlot);
	if (Result)
	{
		return *Result;
	}

	return FText();
}

void UAgoraUIBlueprintFunctionLibrary::GetEnemyColor(FLinearColor& OutEnemyColor)
{
    OutEnemyColor = UAgoraBlueprintFunctionLibrary::GetGlobals()->GetColorFor("Enemy", false);
}

void UAgoraUIBlueprintFunctionLibrary::GetFriendlyColor(FLinearColor& OutFriendlyColor)
{
    OutFriendlyColor = UAgoraBlueprintFunctionLibrary::GetGlobals()->GetColorFor("Friendly", false);
}

void UAgoraUIBlueprintFunctionLibrary::GetBrokenColor(FLinearColor& OutBrokenColor)
{
    OutBrokenColor = UAgoraBlueprintFunctionLibrary::GetGlobals()->GetColorFor("Broken", false);
}

void UAgoraUIBlueprintFunctionLibrary::MakeGradient(const FLinearColor& BaseColor, FLinearColor& OutGradientHigh, FLinearColor& OutGradientLow, float GradientHighMultiplier, float GradientLowMultiplier)
{
    float BaseH, BaseS, BaseV, BaseA = 0.0f;
    UKismetMathLibrary::RGBToHSV(BaseColor, BaseH, BaseS, BaseV, BaseA);

    float GradientHighV = 0.0f, GradientLowV = 0.0f;
    GradientHighV = (BaseV*GradientHighMultiplier);
    GradientLowV = (BaseV*GradientLowMultiplier);

    OutGradientHigh = UKismetMathLibrary::HSVToRGB(BaseH, BaseS, GradientHighV, BaseA);
    OutGradientLow = UKismetMathLibrary::HSVToRGB(BaseH, BaseS, GradientLowV, BaseA);
}
