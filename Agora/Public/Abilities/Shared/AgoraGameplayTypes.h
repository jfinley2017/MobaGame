// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "AgoraGameplayTypes.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FAgoraAbilityMetadata : public FTableRowBase
{
	GENERATED_BODY()
public:
	FAgoraAbilityMetadata() : DisplayName(FText()), Description(FText()), Icon(nullptr)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityUI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityUI")
	FText Description;

	// Icon to be used for hero abilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityUI")
	UTexture2D* Icon;
};
