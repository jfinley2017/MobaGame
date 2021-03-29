// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "PredictedAbilityActor.h"

bool APredictedAbilityActor::IsNetRelevantFor(const AActor * RealViewer, const AActor * ViewTarget, const FVector & SrcLocation) const
{
	Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
	return !IsOwnedBy(ViewTarget);
}
