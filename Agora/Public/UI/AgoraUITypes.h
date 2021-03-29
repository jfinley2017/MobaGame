// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h" // Needed for UStruct/etc
#include "UObject/Object.h" // Needed for UStruct/etc
#include "UObject/Class.h" // Needed for UStruct/etc
#include "Templates/SubclassOf.h" // Needed for UStruct/etc
#include "Engine/NetSerialization.h" // Needed for UStruct/etc

#include "Materials/MaterialInstance.h"
#include "Styling/SlateBrush.h"
#include "Engine/Datatable.h"
#include "AgoraUITypes.generated.h"

/**
 * Data for associating colors with gameplay elements. Eg. "Enemy" is red, "Friendly" is blue.
 */
USTRUCT(BlueprintType)
struct FUIColorData : public FTableRowBase
{
    GENERATED_BODY()

public:

    FUIColorData()    
    {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIColorData")
    FLinearColor IdentifyingColor;

};

/**
 * Data associated with WBP_Ruler, determines the width and sharpness of each tick.
 * Should be broken up (eg, just a concept of TickWidth and TickSharpness and no knowledge of large versus small ticks)
 */
USTRUCT(BlueprintType)
struct FProgressBarTickSettings
{
    GENERATED_BODY()

public:
    FProgressBarTickSettings()
    {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIBarData")
    float LargeTickWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIBarData")
    float LargeTickSharpness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIBarData")
    float SmallTickWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIBarData")
    float SmallTickSharpness;

};

/**
 * Determines whether or not our progress bar uses a solid color or a gradient
 */
UENUM(BlueprintType)
enum class EProgressBarColorSetting : uint8
{
    SingleColor,
    Gradient
};

/**
 * Fills in a generic progress bar, if the ColorSetting is SingleColor then only the SingleColor FLinearColor will be used
 */
USTRUCT(BlueprintType)
struct FUIProgressBarColorSetting
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProgressBarColorSetting")
    EProgressBarColorSetting ColorSetting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProgressBarColorSetting")
    FLinearColor SingleColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProgressBarColorSetting")
    FLinearColor GradientHigh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProgressBarColorSetting")
    FLinearColor GradientLow;

};

/**
 * Allows clients to specify which material they wish to use as a progressbar. 
 */
USTRUCT(BlueprintType)
struct FUIProgressBarStyle
{
    GENERATED_BODY()

public:
    FUIProgressBarStyle() {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProgressBarSettings")
    UMaterialInterface* FillMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProgressBarSettings")
    FUIProgressBarColorSetting Color;

};