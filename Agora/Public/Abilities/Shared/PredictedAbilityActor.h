#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PredictedAbilityActor.generated.h"

UCLASS()
class AGORA_API APredictedAbilityActor : public AActor
{
	GENERATED_BODY()
	
public:	
	//prevent the drone from spawning twice on the owning client (if desired)
	bool IsNetRelevantFor(const AActor * RealViewer, const AActor * ViewTarget, const FVector & SrcLocation) const override;
};
