#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AgoraCharacterBase.h"
#include "AgoraTypes.h"
#include "AgoraGameplayAbility.h" // TSubclassOf<UAgoraGameplay> didn't work without this, which seems strange
#include "AgoraAbilitySystemComponent.generated.h"

class UAgoraAbilitySet;

UENUM(BlueprintType)
enum class EAbilityFailLevelupReason : uint8
{
    NotMetLevelRequirements,
    NoUnspentLevels
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityLevelUpSignature, TSubclassOf<UGameplayAbility>, Ability, int32, NewAbilityLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAbilityFailLevelupSignature, TSubclassOf<UGameplayAbility>, Ability, int32, NewAbilityLevel, EAbilityFailLevelupReason, FailureReason);

UCLASS(ClassGroup = (Agora), meta = (BlueprintSpawnableComponent))
class AGORA_API UAgoraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	
    UPROPERTY(BlueprintAssignable, Category = "Abilities")
    FOnAbilityLevelUpSignature OnAbilityLevelup;

    UPROPERTY(BlueprintAssignable, Category = "Abilities")
    FOnAbilityFailLevelupSignature OnAbilityFailedLevelup;

	// Apply the startup gameplay abilities and effects
	void AddStartupGameplayAbilities();

    UFUNCTION(BlueprintCallable, Category = "Abilities|Slotted")
    bool CanUpgradeAbility(EAbilityInput AbilityInputId);

	// This is called with direct key input
	UFUNCTION(BlueprintCallable, Category = "Abilities|Slotted")
	void UpgradeGameplayAbility(EAbilityInput AbilitySlot);

    UFUNCTION(BlueprintPure, Category = "Abilities|Slotted")
    TSubclassOf<UAgoraGameplayAbility> GetAbilityClassFromInputID(EAbilityInput AbilitySlot);

	// This cannot be exposed to BP because a pointer to enum is not allowed
	const EAbilityInput* GetInputIDFromAbilityClass(TSubclassOf<UAgoraGameplayAbility> AbilityClass);

	UFUNCTION(BlueprintPure, Category = "Abilities|Slotted")
	FGameplayTag GetCooldownTagFromClass(TSubclassOf<UAgoraGameplayAbility> AbilityClass);

	UFUNCTION(BlueprintCallable, Category = "Abilities|Slotted")
	bool TryActivateAbilityBySlot(EAbilityInput AbilitySlot);

    UFUNCTION(BlueprintCallable, Category = "Abilities")
    int32 GetAbilityLevelFromClass(TSubclassOf<UGameplayAbility> AbilityClass);

    /* Apply the Unit's Base Stats in the CurveTable */
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    void SetBaseStatsFromCurveTable(int32 Level, UCurveTable* StatCurveTable);

	UFUNCTION(Client, Reliable, Category = "Abilities")
	void ApplySlowOnClient(TSubclassOf<UGameplayEffect> SlowEffectClass);

    // Level up stats
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization|Abilities")
    UCurveTable* LevelStatsCurveTable;

protected:

    //These need to be overridden for autoattack chaining
    virtual void AbilityLocalInputPressed(int32 InputID) override;
    virtual void AbilityLocalInputReleased(int32 InputID) override;

    UFUNCTION(Client, Reliable, Category = "Abilities")
    void Client_NotifyAbilityLeveledup(TSubclassOf<UGameplayAbility> Ability, int32 NewLevel);
    UFUNCTION(Client, Reliable, Category = "Abilities")
    void Client_NotifyAbilityFailedLevelup(TSubclassOf<UGameplayAbility> Ability, int32 NewLevel, EAbilityFailLevelupReason Reason);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora")
    UAgoraAbilitySet* AbilitySet;

    // Passive gameplay effects applied on creation 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Agora")
    TArray<TSubclassOf<UGameplayEffect>> PassiveStatGameplayEffects;

	// Set of levels that have already had their stats upgraded
	// Extra layer of protection in case some logic error triggers set base stats twice for same level
	TSet<int32> LevelsAlreadySet;
  
    // Handles for applied stat gameplay effects 
    TArray<FActiveGameplayEffectHandle> PassiveStatHandles;
  
    

	

	//for autoattack chaining
	int32 AbilityPrimaryInputID = static_cast<int32>(EAbilityInput::AbilityPrimary);
	UPROPERTY()
	FTimerHandle BasicAttackChainingTimer;
};
