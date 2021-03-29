#pragma once

#include "CoreMinimal.h"
#include "Components/AgoraLevelComponent.h"
#include "GameplayEffectTypes.h" // FOnAttributeChangeData
#include "AgoraXpLevelComponent.generated.h"

UCLASS(ClassGroup = (Agora), meta = (BlueprintSpawnableComponent))
class AGORA_API UAgoraXpLevelComponent : public UAgoraLevelComponent
{
	GENERATED_BODY()
	
public:
    
    // UAgoraLevelComponent interface
    virtual float GetPercentageTowardsNextLevel() override;
    // ~UAgoraLevelComponent interface

    void NotifyExperienceChanged(const FOnAttributeChangeData& XpChangeData);

protected:

	virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraExperienceLevelComponent")
    void ReceiveExperienceChanged(float NewExperienceValue);

	UPROPERTY(BlueprintReadOnly)
	int32 ExperienceToPreviousLevel = 0;
    
    /** 
     * Maps level to experience required to levelup. If index [0] (level 1) is 100, then the player needs 100 TOTAL experience to reach the next level  
     * (which in this case would be level 2).
     */
    TArray<int32> ExperienceForLevel;

    UPROPERTY(BlueprintReadOnly, Category = "AgoraExperienceLevelComponent")
    UAbilitySystemComponent* OwningAbilitySystemComponent = nullptr;

};
