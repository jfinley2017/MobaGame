// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.


#include "GATargetActorStaticBox.h"
#include "AgoraGameplayAbility.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

AGATargetActorStaticBox::AGATargetActorStaticBox()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGATargetActorStaticBox::StartTargeting(UGameplayAbility* Ability)
{
	OwningAbility = Ability;
	MasterPC = Cast<APlayerController>(Ability->GetOwningActorFromActorInfo()->GetInstigatorController());
}

void AGATargetActorStaticBox::ConfirmTargetingAndContinue()
{
	TArray<TWeakObjectPtr<AActor>> OverlappedActors;
	TArray<AActor*> OutHitActors;
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(OwningAbility->GetOwningActorFromActorInfo());

	UAgoraSweepLibrary::AgoraSweepMultiBox(this, MasterPC->GetPawn(), AgoraQueryParams, HitBoxExtent, GetLocation(), GetRotation(), OutHitActors, IgnoredActors);

	for (AActor* Actor : OutHitActors)
	{
		OverlappedActors.Add(Actor);
	}
	
	if (OverlappedActors.Num() > 0)
	{
		FGameplayAbilityTargetDataHandle TargetData = StartLocation.MakeTargetDataHandleFromActors(OverlappedActors);
		TargetDataReadyDelegate.Broadcast(TargetData);
	}
	else
	{
		TargetDataReadyDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
	}
}

void AGATargetActorStaticBox::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (DrawHitbox)
	{
		DrawDebugBox(GetWorld(), GetLocation(), HitBoxExtent, GetRotation().Quaternion(), FColor::Blue, false, -1.f, 0, 1.f);
	}
}


FVector AGATargetActorStaticBox::GetLocation()
{
	FVector BoxLocation;
	if (LocationType == ETargetActorLocationType::LocationRelative)
		BoxLocation = MasterPC->GetPawn()->GetActorLocation() + MasterPC->GetPawn()->GetActorForwardVector() + UKismetMathLibrary::GreaterGreater_VectorRotator(RelativeHitBoxCenter, MasterPC->GetPawn()->GetActorRotation());
	else if (LocationType == ETargetActorLocationType::SocketAttachment)
		BoxLocation = OwningAbility->GetOwningComponentFromActorInfo()->GetSocketLocation(SocketName);
	return BoxLocation;
}

FRotator AGATargetActorStaticBox::GetRotation()
{
	FRotator BoxRotation;
	FVector Location; //throwaway
	MasterPC->GetPlayerViewPoint(Location, BoxRotation);

	BoxRotation.Pitch = 0;
	BoxRotation.Roll = 0;
	return BoxRotation;
}
