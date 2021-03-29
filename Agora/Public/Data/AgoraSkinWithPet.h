// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Data/AgoraSkin.h"
#include "AgoraSkinWithPet.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraSkinWithPet : public UAgoraSkin
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AgoraSkin")
	TSoftObjectPtr<USkeletalMesh> PetSkinMesh = nullptr;
};
