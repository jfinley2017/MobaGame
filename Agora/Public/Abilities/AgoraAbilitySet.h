// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "AgoraTypes.h"
#include "Engine/DataAsset.h"

#include "AgoraAbilitySet.generated.h"

class UAbilitySystemComponent;
class UAgoraGameplayAbility;


USTRUCT()
struct FAgoraAbilityStartupMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Agora")
    EAbilityInput Input;

    UPROPERTY(EditAnywhere, Category = "Agora")
    TSubclassOf<UAgoraGameplayAbility> AbilityClass;

    UPROPERTY(EditAnywhere, Category = "Agora")
    int32 StartingLevel = 0;

};

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraAbilitySet : public UDataAsset
{
	GENERATED_BODY()
	
	public:

    UPROPERTY(EditAnywhere, Category = "Agora")
    TArray<FAgoraAbilityStartupMapping> Abilities;

    void GiveAbilities(UAbilitySystemComponent* ASC) const;
};
	
