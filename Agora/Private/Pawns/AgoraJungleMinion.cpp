// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraJungleMinion.h"
#include "AgoraJungleCamp.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraDataSingleton.h"

AAgoraJungleMinion::AAgoraJungleMinion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
	SetTeam(ETeam::Neutral);
}

void AAgoraJungleMinion::BeginPlay()
{
	Super::BeginPlay();

	OriginalSpawnTransform = GetActorTransform();
}

void AAgoraJungleMinion::FullHeal()
{
	TSubclassOf<UGameplayEffect> Effect = UAgoraBlueprintFunctionLibrary::GetGlobals()->FullHealResetEffect;
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(Effect, 1, GetAbilitySystemComponent()->MakeEffectContext());
	
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}

void AAgoraJungleMinion::FinishReset()
{
	SetActorTransform(OriginalSpawnTransform);
	bIsResetting = false;

	Camp->NotifyFinishResetCamp(this);
}

void AAgoraJungleMinion::StartReset()
{
	if (bIsResetting)
	{
		return;
	}

	bIsResetting = true;

	Camp->NotifyStartResetCamp();
}

void AAgoraJungleMinion::SetTarget(AActor* InTarget)
{
	Target = InTarget;
	OnTargetUpdated.Broadcast(InTarget);
}

ETeam AAgoraJungleMinion::GetTeam() const
{
	return ETeam::Neutral;
}
