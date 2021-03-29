// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraAbilityEffectArea.h"
#include "AbilitySystemComponent.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AbilitySystemInterface.h"
#include "TimerManager.h"

void AAgoraAbilityEffectArea::BeginPlay()
{
	Super::BeginPlay();

	if (!Effect.IsValid()) {
		Collider->Deactivate();
		return;
	}

	Duration = Duration < 0 ? 0 : Duration; // prevent negative durations from messing up the setlifespan call

	SetupOverlaps();

	if (!bIsInfinite)
	{
		SetLifeSpan(Duration + 0.2f); // give a buffer for weirdness/lag 
	}

    
}

void AAgoraAbilityEffectArea::ApplyEffectToActor(AActor* OtherActor)
{
	IAbilitySystemInterface* AbilityFace = Cast<IAbilitySystemInterface>(OtherActor);
	if (AbilityFace)
	{
		ensureMsgf(EffectCauser, TEXT("ApplyEffectToActor requires instigator"));
		if (QueryParams.MatchesQuery(EffectCauser, OtherActor)) 
		{
			UAbilitySystemComponent* ASC = AbilityFace->GetAbilitySystemComponent();
			ActiveEffects.Add(ASC, ASC->ApplyGameplayEffectSpecToSelf(*Effect.Data, FPredictionKey()));
			
			OnApplyEffectToActor(OtherActor);
		}
	}
}

void AAgoraAbilityEffectArea::RemoveEffectsFromActorASC(UAbilitySystemComponent* ASC)
{
	FActiveGameplayEffectHandle* EffectHandlePtr = ActiveEffects.Find(ASC);

	if (EffectHandlePtr)
	{
		ASC->RemoveActiveGameplayEffect(*EffectHandlePtr);
	}
}

void AAgoraAbilityEffectArea::SetupOverlaps()
{
	
	if (Collider)
	{
		Collider->OnComponentBeginOverlap.AddDynamic(this, &AAgoraAbilityEffectArea::HandleAreaOverlap);
		Collider->OnComponentEndOverlap.AddDynamic(this, &AAgoraAbilityEffectArea::HandleAreaOverlapEnd);
	}

	if (!bIsInfinite)
	{
		SetupAreaEnd();
	}
}

void AAgoraAbilityEffectArea::SetupAreaEnd()
{
	FTimerHandle EffectTimer;
	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([&]()
	{
		Collider->OnComponentBeginOverlap.RemoveDynamic(this, &AAgoraAbilityEffectArea::HandleAreaOverlap);
		Collider->OnComponentEndOverlap.RemoveDynamic(this, &AAgoraAbilityEffectArea::HandleAreaOverlapEnd);

		if (bRemoveEffectOnEnd)
		{
			TArray<UAbilitySystemComponent*> ASCs;
			ActiveEffects.GetKeys(ASCs);
			for (UAbilitySystemComponent* ASC : ASCs)
			{
				// Stop applying the effect after the duration is over
				RemoveEffectsFromActorASC(ASC); // only removes if bRemoveEffectOnEnd is set
			}
		}
	});

	if (!bIsInfinite)
	{
		GetWorld()->GetTimerManager().SetTimer(EffectTimer, TimerCallback, Duration, false);
	}
}

// This is only setup in the case of duration > 0
void AAgoraAbilityEffectArea::HandleAreaOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	ApplyEffectToActor(OtherActor);
}

void AAgoraAbilityEffectArea::HandleAreaOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bRemoveEffectOnEnd)
	{
		IAbilitySystemInterface* AbilityFace = Cast<IAbilitySystemInterface>(OtherActor);
		if (AbilityFace)
		{
			UAbilitySystemComponent* ASC = AbilityFace->GetAbilitySystemComponent();
			RemoveEffectsFromActorASC(ASC);
		}
	}
}
