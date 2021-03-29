// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "GATargetActorLine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "Agora.h"

AGATargetActorLine::AGATargetActorLine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
}

FHitResult AGATargetActorLine::PerformTrace(AActor* InSourceActor)
{
	TArray<AActor*> ActorsToIgnore;

	ActorsToIgnore.Add(InSourceActor);
	ActorsToIgnore.Add(MasterPC->GetPawn());

	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	Params.bReturnPhysicalMaterial = false;
	Params.AddIgnoredActors(ActorsToIgnore);

	FVector TraceStart = InSourceActor->GetActorLocation();

	FVector ViewPoint;
	FRotator ViewRotation;
	MasterPC->GetPlayerViewPoint(ViewPoint, ViewRotation);
	FVector TraceEnd = TraceStart + (ViewRotation.Vector() * MaxRange);
	FHitResult ReturnHitResult;
	InSourceActor->GetWorld()->LineTraceSingleByObjectType(ReturnHitResult, TraceStart, TraceEnd, FCollisionObjectQueryParams(TraceObjectTypes), Params);
	//Default to end of trace line if we don't hit anything.
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
	}
	if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
	{
		const bool bHitActor = (ReturnHitResult.bBlockingHit && (ReturnHitResult.Actor != NULL));
		const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? ReturnHitResult.Actor->GetActorLocation() : ReturnHitResult.Location;

		LocalReticleActor->SetActorLocation(ReticleLocation);
		LocalReticleActor->SetIsTargetAnActor(bHitActor);
	}

	if (bDebug)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green);
	}
	return ReturnHitResult;
}