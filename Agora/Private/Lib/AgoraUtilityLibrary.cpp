// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraUtilityLibrary.h"

float UAgoraUtilityLibrary::StdGaussian()
{
	float u1 = 1.0f - FMath::FRand();
	float u2 = 1.0f - FMath::FRand();
	float Gaussian = FMath::Sqrt(-2.0 * FMath::Loge(u1)) * FMath::Sin(2.0 * PI * u2);
	return Gaussian;
}

FString UAgoraUtilityLibrary::RandomString(int32 Length, bool bAlphanumeric /*= false*/)
{
	FString Source = bAlphanumeric ? "0123456789abcdefghijklmnopqrstuvwxyz" : "abcdefghijklmnopqrstuvwxyz";
	FString Rand;
	for (int Idx = 0; Idx < Length; ++Idx)
	{
		Rand.AppendChar(Source[FMath::RandHelper(Length)]);
	}
	return Rand;
}
