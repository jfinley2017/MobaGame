// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AN_Footstep.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ConstructorHelpers.h"
#include "Sound/SoundCue.h"
#include "Landscape.h"
#include "Components/AudioComponent.h"
#include "Agora.h"
#include "Engine.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UAN_Footstep::UAN_Footstep()
{
	Sound = ConstructorHelpers::FObjectFinder<USoundCue>(TEXT("SoundCue'/Game/Sounds/Footsteps/SC_Footstep.SC_Footstep'")).Object;
}

void UAN_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AActor* Owner = MeshComp->GetOwner();
	FVector TraceStart = Owner->GetActorLocation();
	FVector TraceEnd = ((-Owner->GetActorUpVector()) * 2000.0f) + TraceStart;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	Params.AddIgnoredActor(Owner);
	Owner->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);
	UAudioComponent* AudioPlayer = UGameplayStatics::SpawnSoundAtLocation(Owner, Sound, HitResult.Location);
	if (AudioPlayer && !AudioPlayer->IsPendingKill())
	{
		EPhysicalSurface SurfaceType;
		if (!HitResult.GetActor())
		{
			SurfaceType = EPhysicalSurface::SurfaceType_Default;
		}
		else
		{
			if (HitResult.GetActor()->IsA(ALandscape::StaticClass()))
			{
				SurfaceType = UAgoraBlueprintFunctionLibrary::GetLandscapeSurfaceTypeFromHitResult(HitResult);
			}
			else
			{
				SurfaceType = UAgoraBlueprintFunctionLibrary::GetStaticMeshSurfaceTypeFromHitResult(HitResult);
			}
		}
		
		switch (SurfaceType)
		{
		case EPhysicalSurface::SurfaceType1: //concrete
			AudioPlayer->SetIntParameter(FName("SurfaceType"), 0);
			break;
		case EPhysicalSurface::SurfaceType2: //grassy
			AudioPlayer->SetIntParameter(FName("SurfaceType"), 1);
			break;
		case EPhysicalSurface::SurfaceType3: //metallic
			AudioPlayer->SetIntParameter(FName("SurfaceType"), 2);
			break;
		}
	}
}