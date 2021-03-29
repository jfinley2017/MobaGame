#include "AgoraXpLevelComponent.h"
#include "AbilitySystemInterface.h"
#include "AttributeSets/AgoraAttributeSetEconomyBase.h"
#include "Pawns/AgoraCharacterBase.h"

void UAgoraXpLevelComponent::BeginPlay()
{
	Super::BeginPlay();

    IAbilitySystemInterface* OwnerAsAbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
    ensure(OwnerAsAbilitySystemInterface);
    OwningAbilitySystemComponent = OwnerAsAbilitySystemInterface->GetAbilitySystemComponent();
    ensure(OwningAbilitySystemComponent);
    FGameplayAttribute ExperienceAttribute = UAgoraAttributeSetEconomyBase::GetExperienceAttribute();
    ensure(OwningAbilitySystemComponent->HasAttributeSetForAttribute(ExperienceAttribute));
    
    OwningAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ExperienceAttribute).AddUObject(this, &UAgoraXpLevelComponent::NotifyExperienceChanged);

    TArray<FName> RowNames = LevelTable->GetRowNames();
	for (FName RowName : RowNames)
	{
		FXpLevelUpData* Row = LevelTable->FindRow<FXpLevelUpData>(RowName, TEXT(""));
		// Don't need to increase level for the initialize row
		if (Row->XpToLevel > 0)
		{
			ExperienceForLevel.Push(Row->XpToLevel);
		}
	}

	// We will rely on the order of the array
	ExperienceForLevel.Sort();
}

float UAgoraXpLevelComponent::GetPercentageTowardsNextLevel()
{
	ensure(OwningAbilitySystemComponent);
	if (CurrentLevel >= MaxLevel)
	{
		return 0;
	}

	FGameplayAttribute ExperienceAttribute = UAgoraAttributeSetEconomyBase::GetExperienceAttribute();
	return (OwningAbilitySystemComponent->GetNumericAttribute(ExperienceAttribute) - ExperienceToPreviousLevel) / (GetTotalXpForNextLevel()->XpToLevel - ExperienceToPreviousLevel);
}

void UAgoraXpLevelComponent::NotifyExperienceChanged(const FOnAttributeChangeData& XpChangeData)
{
	float NewExperience = XpChangeData.NewValue;

    ReceiveExperienceChanged(NewExperience);

    // Handles potentially leveling up more than once in a single call
    while (ExperienceForLevel.Num() > 0 && NewExperience >= ExperienceForLevel[0])
    {
        ExperienceToPreviousLevel = ExperienceForLevel[0];
        ExperienceForLevel.RemoveAt(0);
        SetLevel(CurrentLevel + 1);
    }
}