// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "GATargetActorPyramid.h"
#include "AgoraGameplayAbility.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "DrawDebugHelpers.h"

AGATargetActorPyramid::AGATargetActorPyramid()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGATargetActorPyramid::StartTargeting(UGameplayAbility* Ability)
{
	OwningAbility = Ability;
	MasterPC = Cast<APlayerController>(Ability->GetOwningActorFromActorInfo()->GetInstigatorController());
}

void AGATargetActorPyramid::ConfirmTargetingAndContinue()
{
	TArray<TWeakObjectPtr<AActor>> OverlappedActors;
	TArray<AActor*> OutHitActors;
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(OwningAbility->GetOwningActorFromActorInfo());

	UAgoraSweepLibrary::AgoraSweepMultiPyramid(this, OwningAbility->GetOwningActorFromActorInfo(), AgoraQueryParams, BaseWidth, 
		BaseHeight, GetLocation(), PyramidHeight, GetRotation(), OutHitActors, IgnoredActors, Spread);

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

void AGATargetActorPyramid::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDrawPyramid)
	{
		FRotator PyramidRotation = GetRotation();
		TArray<FVector> EdgePoints;
		EdgePoints.Emplace(FVector(PyramidHeight, BaseWidth / 2, 0.0f));
		EdgePoints.Emplace(FVector(PyramidHeight, -BaseWidth / 2, 0.0f));
		EdgePoints.Emplace(FVector(PyramidHeight, 0.0f, BaseHeight / 2));
		EdgePoints.Emplace(FVector(PyramidHeight, 0.0f, -BaseHeight / 2));
		for (const FVector& Point : EdgePoints)
		{
			FVector RotatedPoint = UKismetMathLibrary::GreaterGreater_VectorRotator(Point, PyramidRotation);
			FVector Location = GetLocation();
			DrawDebugLine(GetWorld(), Location, Location + RotatedPoint, FColor::Red);
		}
	}
}

FVector AGATargetActorPyramid::GetLocation()
{
	FVector PyramidLocation;
	if (LocationType == ETargetActorLocationType::LocationRelative)
		PyramidLocation = MasterPC->GetPawn()->GetActorLocation() + MasterPC->GetPawn()->GetActorForwardVector() + UKismetMathLibrary::GreaterGreater_VectorRotator(RelativePyramidOrigin, MasterPC->GetPawn()->GetActorRotation());
	else if (LocationType == ETargetActorLocationType::SocketAttachment)
		PyramidLocation = OwningAbility->GetOwningComponentFromActorInfo()->GetSocketLocation(SocketName);
	return PyramidLocation;
}

FRotator AGATargetActorPyramid::GetRotation()
{
	FRotator PyramidRotation;
	FVector Location; //throwaway
	UAgoraBlueprintFunctionLibrary::GetPlayerViewLocationAndRotation(Cast<AAgoraCharacterBase>(MasterPC->GetPawn()), Location, PyramidRotation);

	return PyramidRotation;
}
