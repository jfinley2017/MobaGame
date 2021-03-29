// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AttributeSets/AgoraAttributeSetBase.h"
#include "AgoraAttributeSetEconomyBase.generated.h"

/**
 * Base class for all "economy" related attributes
 */
UCLASS()
class AGORA_API UAgoraAttributeSetEconomyBase : public UAgoraAttributeSetBase
{
	GENERATED_BODY()
	
public:

    UAgoraAttributeSetEconomyBase();

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;

    //////////////////////////////////////////////////////////////////////////
    // Economy Attributes

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_GoldKillReward, BlueprintReadWrite, Category = "AttributeSetEconomy")
	FGameplayAttributeData GoldKillReward;
	ATTRIBUTE_ACCESSORS(UAgoraAttributeSetEconomyBase, GoldKillReward);
	UFUNCTION()
	void OnRep_GoldKillReward();

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_ExperienceKillReward, BlueprintReadWrite, Category = "AttributeSetEconomy")
	FGameplayAttributeData ExperienceKillReward;
	ATTRIBUTE_ACCESSORS(UAgoraAttributeSetEconomyBase, ExperienceKillReward);
	UFUNCTION()
	void OnRep_ExperienceKillReward();

    /** Gold, primary economy source */
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Gold, BlueprintReadWrite, Category = "AttributeSetEconomy")
    FGameplayAttributeData Gold;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetEconomyBase, Gold);
    UFUNCTION()
    void OnRep_Gold();

    /** Experience, primary leveling source */
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Experience, BlueprintReadWrite, Category = "AttributeSetEconomy")
    FGameplayAttributeData Experience;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetEconomyBase, Experience);
    UFUNCTION()
    void OnRep_Experience();

    /** Unspent level tokens */
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_UnspentLevelPoints, BlueprintReadWrite, Category = "AttributeSetEconomy")
    FGameplayAttributeData UnspentLevelPoints;
    ATTRIBUTE_ACCESSORS(UAgoraAttributeSetEconomyBase, UnspentLevelPoints);
    UFUNCTION()
    void OnRep_UnspentLevelPoints();

    /*   UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CurrentLevelExperienceCap, BlueprintReadWrite, Category = "AttributeSetEconomy")
       FGameplayAttributeData CurrentLevelExperienceCap;
       ATTRIBUTE_ACCESSORS(UAgoraAttributeSetEconomyBase, CurrentLevelExperienceCap);
       UFUNCTION()
       void OnRep_CurrentLevelExperienceCap();
   */

};
