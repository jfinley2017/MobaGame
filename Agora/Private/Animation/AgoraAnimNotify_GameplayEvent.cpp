// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraAnimNotify_GameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

void UAgoraAnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::Notify(MeshComp, Animation);

    for (FGameplayTag& Tag : NotifyEventTags)
    {
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), Tag, FGameplayEventData());
    }

}
