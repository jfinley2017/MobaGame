// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AgoraHeroMetadata.generated.h"

class UAgoraSkin;
class AAgoraCharacterBase;
class UAnimSequence;
class UAnimMontage;
class UAnimInstance;
class UTexture2D;

/**
 * Anything associated to the hero that is not gameplay related, helps reduce bloat in character
 */
UCLASS(BlueprintType)
class AGORA_API UAgoraHeroMetadata : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroMetadata")
	TArray<UAgoraSkin*> Skins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroMetadata")
	TSoftClassPtr<AAgoraCharacterBase> HeroClass;

	// So that we can play level start anims
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroMetadata")
	TSubclassOf<UAnimInstance> AnimClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroMetadata")
	UAnimMontage* OnHeroSelectAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroMetadata")
	UAnimSequence* HeroIdleAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroMetadata")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroMetadata")
    UTexture2D* HeroPortrait = nullptr;
};
