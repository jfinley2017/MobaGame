// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#include "AgoraMaterialEffectsComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/MeshComponent.h"

UAgoraMaterialEffectsComponent::UAgoraMaterialEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAgoraMaterialEffectsComponent::BeginPlay()
{
	Super::BeginPlay();
	SetMeshMaterialsDynamic();
}



void UAgoraMaterialEffectsComponent::TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);

	// MF_HitFlash
	// Possibly replace this with something more abstract that can do different colors and such, maybe use timelines
	if (DamageFlashPercent > 0)
	{
		SetAllMaterialValue("Damage", DamageFlashPercent);
		DamageFlashPercent -= HitFlashDecreaseStepSize * DeltaSeconds;

		if (DamageFlashPercent <= 0) // avoids getting stuck at 0.1
		{
			DamageFlashPercent = 0.0f;
			SetAllMaterialValue("Damage", DamageFlashPercent);
		}
	}

	if (DisintegratePercent > 0)
	{
		// MF_DeathFade
		SetAllMaterialValue("FadeOut", DisintegratePercent);
		DisintegratePercent -= 0.05f * DeltaSeconds;

		if (DisintegratePercent <= 0) // avoids getting stuck at 0.1
		{
			DisintegratePercent = 0.0f;
			SetAllMaterialValue("FadeOut", DisintegratePercent);
		}
	}
}

void UAgoraMaterialEffectsComponent::SetMeshMaterialsDynamic()
{
	OuterMesh = Cast<AActor>(GetOuter())->FindComponentByClass<UMeshComponent>();

	// Turn all materials into dynamic instanced materials
	TArray<UMaterialInterface*> Mats = OuterMesh->GetMaterials();
	for (uint8 Idx = 0; Idx < Mats.Num(); Idx++)
	{
		if (!Cast<UMaterialInstanceDynamic>(OuterMesh->GetMaterial(Idx)))
		{
			OuterMesh->SetMaterial(Idx, UMaterialInstanceDynamic::Create(Mats[Idx], this));
		}
	}
}

void UAgoraMaterialEffectsComponent::PlayHitFlash(float Intensity /*= 1.0f*/)
{
	DamageFlashPercent = Intensity;
}

void UAgoraMaterialEffectsComponent::PlayDeathDisintegrate()
{
	DisintegratePercent = 1.0f;
}

void UAgoraMaterialEffectsComponent::SetAllMaterialValue(FName FnName, float Val)
{
	for (UMaterialInterface* MatI : OuterMesh->GetMaterials())
	{
		Cast<UMaterialInstanceDynamic>(MatI)->SetScalarParameterValue(FnName, Val);
	}
}
