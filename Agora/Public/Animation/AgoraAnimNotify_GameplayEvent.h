// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AgoraAnimNotify_GameplayEvent.generated.h"


/**
 * 
 */
UCLASS()
class AGORA_API UAgoraAnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
	
	//AnimNotify
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
    // ~AnimNotify
    
    /** Tags to be fired when this anim notify is triggered */
    UPROPERTY(EditAnywhere, Category = "Agora")
    TArray<FGameplayTag> NotifyEventTags;

};
