// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AgoraSkin.generated.h"

class USkeletalMesh;

/**
 * This data asset exists anticipating there being more things that will be skinned in the future besides just meshes
 */
UCLASS(BlueprintType)
class AGORA_API UAgoraSkin : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AgoraSkin")
	TSoftObjectPtr<USkeletalMesh> SkinMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AgoraSkin")
	FText SkinName;
};
