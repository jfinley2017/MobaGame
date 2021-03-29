// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Gameframework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "AgoraTypes.h"
#include "AgoraGameplayTags.generated.h"

#if defined(__clang__) //If compiling for Linux
#define IS_DESCR_FN(Name) static bool Is##Name(AActor* Act) { return UAgoraGameplayTags::IsDescriptorType(Act, UAgoraGameplayTags::Name()); }
#else
#define IS_DESCR_FN(Name) static bool Is##Name(AActor* Act) { return UAgoraGameplayTags::IsDescriptorType(Act, UAgoraGameplayTags::##Name()); }
#endif

/**
 * Frequently used tags. Use these rather than requesting normally so that changes are not hard to make later.
 */
UCLASS()
class AGORA_API UAgoraGameplayTags : public UObject
{
	GENERATED_BODY()
	
private:
	static bool IsDescriptorType(AActor* Act, FGameplayTag Tag) {
		IGameplayTagAssetInterface* TagActor = Cast<IGameplayTagAssetInterface>(Act);
		if (TagActor) {
			return TagActor->HasMatchingGameplayTag(Tag);
		}
		return false;
	}

public:

	IS_DESCR_FN(Hero)
	IS_DESCR_FN(Core)
	IS_DESCR_FN(Tower)
	IS_DESCR_FN(Inhibitor)
	IS_DESCR_FN(Minion)
	IS_DESCR_FN(JungleCreep)

	// Spectator is special because I was too lazy to implement the IGameplayTagAssetInterface - It's a normal tag
	static bool IsSpectator(const AActor* Actor) {
		if (Actor)
		{
            if (Cast<AController>(Actor))
            {
                return Cast<AController>(Actor)->GetPawn() && Cast<AController>(Actor)->GetPawn()->ActorHasTag(FName("Spectator"));
            }
			return Actor->Tags.Contains(FName("Spectator"));
		}
		return false;
	}

	static FName Spectator() { return FName("Spectator"); }

	static FGameplayTag GetAbilitySlotCooldownTag(EAbilityInput AbilitySlot) {
		switch (AbilitySlot) {
		case EAbilityInput::AbilityPrimary:
			break;
			//return FGameplayTag::RequestGameplayTag("Cooldown.Ability.LMB");
		case EAbilityInput::AbilityAlternate:
			return FGameplayTag::RequestGameplayTag("Cooldown.Ability.RMB");
		case EAbilityInput::AbilityOne:
			return FGameplayTag::RequestGameplayTag("Cooldown.Ability.Q");
        case EAbilityInput::AbilityTwo:
			return FGameplayTag::RequestGameplayTag("Cooldown.Ability.E");
		case EAbilityInput::AbilityUltimate:
			return FGameplayTag::RequestGameplayTag("Cooldown.Ability.R");
		default:
			break;
		}

		return FGameplayTag::EmptyTag;
	}

	static FGameplayTag Core() { return FGameplayTag::RequestGameplayTag("Descriptor.Unit.Tower.Core"); }
	static FGameplayTag Hero() { return FGameplayTag::RequestGameplayTag("Descriptor.Unit.Hero"); }
	static FGameplayTag Tower() { return FGameplayTag::RequestGameplayTag("Descriptor.Unit.Tower"); }
	static FGameplayTag Inhibitor() { return FGameplayTag::RequestGameplayTag("Descriptor.Unit.Tower.Inhibitor"); }
	static FGameplayTag Minion() { return FGameplayTag::RequestGameplayTag("Descriptor.Unit.Minion"); }
	static FGameplayTag JungleCreep() { return FGameplayTag::RequestGameplayTag("Descriptor.Unit.Minion.Jungle"); }

	static FGameplayTag RequestDeath() { return FGameplayTag::RequestGameplayTag("Events.RequestDeath"); }
	static FGameplayTag Death() { return FGameplayTag::RequestGameplayTag("Events.Death"); }
	static FGameplayTag KilledActor() { return FGameplayTag::RequestGameplayTag("Events.KilledActor"); }

	static FGameplayTag DamageInstigated() { return FGameplayTag::RequestGameplayTag("Events.DamageInstigated"); }
	static FGameplayTag DamageReceived() { return FGameplayTag::RequestGameplayTag("Events.DamageReceived"); }

	static FGameplayTag DamageTypePhysical() { return FGameplayTag::RequestGameplayTag("Descriptor.DamageType.Physical"); }
	static FGameplayTag DamageTypeMagical() { return FGameplayTag::RequestGameplayTag("Descriptor.DamageType.Magical"); }
	static FGameplayTag DamageTypeTrue() { return FGameplayTag::RequestGameplayTag("Descriptor.DamageType.True"); }

	static FGameplayTag CriticalHit() { return FGameplayTag::RequestGameplayTag("Descriptor.DamageType.CriticalHit"); }
	static FGameplayTag CanCrit() { return FGameplayTag::RequestGameplayTag("Descriptor.Ability.CanCriticallyHit"); }
	static FGameplayTag Charges() { return FGameplayTag::RequestGameplayTag("Descriptor.Ability.Charges"); }

	static FGameplayTag SetByCallerBaseDamage() { return FGameplayTag::RequestGameplayTag("SetByCaller.BaseDamage"); }
	static FGameplayTag SetByCallerScalingValue() { return FGameplayTag::RequestGameplayTag("SetByCaller.ScalingValue"); }
	static FGameplayTag SetByCallerDuration() { return FGameplayTag::RequestGameplayTag("SetByCaller.Duration"); }

	static FGameplayTag Targeting() { return FGameplayTag::RequestGameplayTag("Skills.Targeting"); }
	static FGameplayTag Casting() { return FGameplayTag::RequestGameplayTag("Skills.Casting"); }

	static FGameplayTag Dead() { return FGameplayTag::RequestGameplayTag("Status.IsDead"); }
	static FGameplayTag Alive() { return FGameplayTag::RequestGameplayTag("Status.IsAlive"); }
};
