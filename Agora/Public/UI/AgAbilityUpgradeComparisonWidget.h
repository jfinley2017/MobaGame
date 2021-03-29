#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AgoraTypes.h"
#include "AgAbilityUpgradeComparisonWidget.generated.h"


USTRUCT(BlueprintType)
struct FAbilityLevelCompareData
{
	GENERATED_BODY()

public:
	FAbilityLevelCompareData() {};

	UPROPERTY(BlueprintReadWrite)
		float OldValue;
	UPROPERTY(BlueprintReadWrite)
		float NewValue;

	UPROPERTY(BlueprintReadWrite)
		FName ModifierName;
};

/**
 * 
 */
UCLASS()
class AGORA_API UAgAbilityUpgradeComparisonWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	EAbilityInput AbilitySlot;

	/**
	 * get all the modifiersByLevel from the ability
	 * figure out which ones actually changed
	 */
	UFUNCTION(BlueprintCallable)
	const TArray<FAbilityLevelCompareData> GetLevelComparisons() const;
};
