#include "AgoraGameplayCueCameraImpact.h"
#include "Kismet/GameplayStatics.h"
#include "Classes/Camera/CameraComponent.h"
#include "AgoraHeroBase.h"

bool UAgoraGameplayCueCameraImpact::HandlesEvent(EGameplayCueEvent::Type EventType) const
{
	return (EventType == EGameplayCueEvent::Executed);
}

void UAgoraGameplayCueCameraImpact::HandleGameplayCue(AActor* HitActor, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	check(EventType == EGameplayCueEvent::Executed);
	check(HitActor);
	
	if (!ParticleSystem) { return; }

	// We only care if we hit an actual player
	AAgoraHeroBase* HitCharacter = Cast<AAgoraHeroBase>(HitActor);
	if (HitCharacter && HitCharacter->IsLocallyControlled())
	{
		UCameraComponent* HitCamera = HitCharacter->GetCameraComponent();

		const FHitResult* HitResult = Parameters.EffectContext.GetHitResult();

		// Flip the direction so you can see it
		FRotator Rot = FRotator(0, 180, 0);
		FVector AdjustOffset = FVector(50, 0, 0);

		UGameplayStatics::SpawnEmitterAttached(ParticleSystem, HitCamera, FName(NAME_None), AdjustOffset, Rot, EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::None);
	}
}