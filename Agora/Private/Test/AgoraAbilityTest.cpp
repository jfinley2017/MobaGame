// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraAbilityTest.h"
#include "AIController.h"
#include "AgoraTypes.h"
#include "AgoraAbilitySystemComponent.h"
#include "AgoraHeroBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AgoraLevelComponent.h"

void AAgoraAbilityTest::UseAbility()
{
	if (AbilitySlot == EAbilityInput::Passive)
	{
		return;
	}

	if (!AssertIsValid(Hero, "Ability test hero"))
	{
		Quit();
		return;
	}

	if (bIsTargeted)
	{
		if (AssertIsValid(AbilityTarget, "Ability test target"))
		{

			AAIController* AI = Hero->GetController<AAIController>();
			if (AI)
			{
				AI->SetFocus(AbilityTarget);
			}
		}
		else
		{
			Quit();
			return;
		}
	}
	
	AcquireAbility();
	Hero->GetAgoraAbilitySystemComponent()->TryActivateAbilityBySlot(AbilitySlot);
}

void AAgoraAbilityTest::AcquireAbility()
{
	Hero->GetLevelComponent()->SetLevel(5);
	// #TODO: Something like this, once we set the abilities to not be all be available at level 1
	//switch (AbilitySlot)
	//{
	//case EAbilityInput::Passive:
	//	break;
	//case EAbilityInput::AbilityPrimary:
	//	break;
	//case EAbilityInput::AbilityAlternate:
	//	Hero->GetLevelComponent()->SetLevel(2);
	//	break;
	//case EAbilityInput::AbilityOne:
	//	Hero->LevelComponent->SetLevel(2);
	//	break;
	//case EAbilityInput::AbilityTwo:
	//	Hero->LevelComponent->SetLevel(2);
	//	break;
	//case EAbilityInput::AbilityUltimate:
	//	Hero->LevelComponent->SetLevel(5);
	//	break;
	//default:
	//	break;
	//}
	Hero->UpgradeAbility(AbilitySlot);
}

void AAgoraAbilityTest::Quit()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void AAgoraAbilityTest::GatherRelevantActors(TArray<AActor*>& OutActors) const
{
	if (Hero)
	{
		OutActors.Add(Hero);
	}

	if (AbilityTarget)
	{
		OutActors.Add(AbilityTarget);
	}

	Super::GatherRelevantActors(OutActors);
}
