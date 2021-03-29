// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.


#include "AgoraAttributeSetEconomyBase.h"
#include "Net/UnrealNetwork.h"

UAgoraAttributeSetEconomyBase::UAgoraAttributeSetEconomyBase():
    Super()
{
    UnspentLevelPoints = 1.0f;
	GoldKillReward = 0.0f;
	ExperienceKillReward = 0.0f;
}

void UAgoraAttributeSetEconomyBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UAgoraAttributeSetEconomyBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data)
{
    Super::PostGameplayEffectExecute(Data);

}

void UAgoraAttributeSetEconomyBase::OnRep_GoldKillReward()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetEconomyBase, GoldKillReward);
}

void UAgoraAttributeSetEconomyBase::OnRep_ExperienceKillReward()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetEconomyBase, ExperienceKillReward);
}

void UAgoraAttributeSetEconomyBase::OnRep_Gold()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetEconomyBase, Gold);
}

void UAgoraAttributeSetEconomyBase::OnRep_Experience()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetEconomyBase, Experience);

}

void UAgoraAttributeSetEconomyBase::OnRep_UnspentLevelPoints()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetEconomyBase, UnspentLevelPoints);
}

//void UAgoraAttributeSetEconomyBase::OnRep_CurrentLevelExperienceCap()
//{
//    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetEconomyBase, CurrentLevelExperienceCap);
//
//}

void UAgoraAttributeSetEconomyBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetEconomyBase, ExperienceKillReward, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetEconomyBase, GoldKillReward, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetEconomyBase, Gold, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetEconomyBase, Experience, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetEconomyBase, UnspentLevelPoints, COND_None, REPNOTIFY_Always);
    //DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetEconomyBase, CurrentLevelExperienceCap, COND_None, REPNOTIFY_Always);


}