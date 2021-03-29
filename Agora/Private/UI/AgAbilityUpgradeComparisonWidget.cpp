#include "AgAbilityUpgradeComparisonWidget.h"
#include "AgoraGameplayAbility.h"
#include "AgoraAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AgoraTypes.h" // FTextureTable
#include "AgoraAbilityLibrary.h"

const TArray<FAbilityLevelCompareData> UAgAbilityUpgradeComparisonWidget::GetLevelComparisons() const
{
	TArray<FAbilityLevelCompareData> UpdatedModifiers;

    const TSubclassOf<UAgoraGameplayAbility> AbilityPtr = UAgoraAbilityLibrary::GetAbilityClassByInputId(GetOwningPlayerPawn(), AbilitySlot);

	if (!AbilityPtr)
	{
		return UpdatedModifiers; // did not find ability
	}

	TSubclassOf<UGameplayAbility> Ability = *AbilityPtr;

	TSubclassOf<UAgoraGameplayAbility> AgAbility = TSubclassOf<UAgoraGameplayAbility>(*Ability);

	if (!AgAbility)
	{
		return UpdatedModifiers; //Only Agora abilities have all the levelup functionality = Empty array
	}

	UAgoraGameplayAbility* AgAbilityCDO = AgAbility.GetDefaultObject();
	// Have to use CDO because abilities aren't necessarily instanced 
	// and who knows how to get the instance even if they were?
	UCurveTable* LevelTable = AgAbilityCDO->GetAbilityLevelTable();

	if (!LevelTable)
	{
		return UpdatedModifiers; // No level data = Empty array
	}

	const TMap<FName, FSimpleCurve*>& RowMap = LevelTable->GetSimpleCurveRowMap();
	int32 CurrentLevel = UAgoraAbilityLibrary::GetAbilityLevelFromInputID(GetOwningPlayerPawn(), AbilitySlot);

	// Each key is the name of the modifier being changed, associated to a curve of that modifier's levelup progression
	for (TPair<FName, FSimpleCurve*> Row : RowMap)
	{
		FScalableFloat AbilityTable = AgAbilityCDO->GetAbilityStatCurve(Row.Key);
		FAbilityLevelCompareData CompareData;

		CompareData.OldValue = AbilityTable.GetValueAtLevel(CurrentLevel);
		CompareData.NewValue = AbilityTable.GetValueAtLevel(CurrentLevel + 1);
		CompareData.ModifierName = Row.Key;

		bool IsHeroLevel = CompareData.ModifierName.ToString() == "heroLevelRequired"; // not ideal

		// Only interested in what has actually changed in a level up
		if (CompareData.OldValue != CompareData.NewValue && !IsHeroLevel)
		{
			UpdatedModifiers.Add(CompareData);
		}
	}

	return UpdatedModifiers;
}
