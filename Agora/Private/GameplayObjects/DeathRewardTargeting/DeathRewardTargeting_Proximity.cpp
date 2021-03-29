// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "DeathRewardTargeting_Proximity.h"
#include "Components/AgoraDeathRewardComponent.h"
#include "AgoraGameplayTags.h"
#include "AgoraBlueprintFunctionLibrary.h"

void UDeathRewardTargeting_Proximity::GetRewardTargets(TArray<AActor*>& OutTargets, FDamageContext Context)
{
	AActor* Killer = Context.DamageInstigator;
	FVector Center = Context.DamagedActor->GetActorLocation();
	float Radius = 1800.0f;

	FAgoraSweepQueryParams QueryParams = FAgoraSweepQueryParams();
	QueryParams.TeamRule = ESweepTeamRule::Allies;
	
	FGameplayTagContainer RequiredTags = FGameplayTagContainer();
	RequiredTags.AddTag(UAgoraGameplayTags::Hero());
	QueryParams.RequiredTags = RequiredTags;
	
	TArray<AActor*> IgnoredActors;

	// This will add the actors to OutTargets
	UAgoraBlueprintFunctionLibrary::AgoraSweepMultiSphere(Killer, Killer, QueryParams, Radius, Center, OutTargets, IgnoredActors);
}
