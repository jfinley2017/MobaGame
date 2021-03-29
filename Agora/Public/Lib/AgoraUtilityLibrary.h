// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AgoraUtilityLibrary.generated.h"

/**
 * Utilities that don't belong in any other category.
 */
UCLASS()
class AGORA_API UAgoraUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Generates a number from a Gaussian distribution with mean 0 and stddev 1. See https://stackoverflow.com/questions/218060/random-gaussian-variables for details.
	UFUNCTION(BlueprintCallable, Category = "AgoraUtilityLibrary")
	static float StdGaussian();

	/**
	  Generates a random string of the specified length.
	  @Param Length the length of the random string
	  @Param bAlphanumeric whether to include letters and numbers, or just letters
	*/
	UFUNCTION(BlueprintCallable, Category = "AgoraUtilityLibrary")
	static FString RandomString(int32 Length, bool bAlphanumeric = false);
};
