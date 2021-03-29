#pragma once

#include "CoreMinimal.h"
#include "Components/AgoraLevelComponent.h"
#include "AgoraTimeLevelComp.generated.h"

UCLASS(ClassGroup = (Agora), meta = (BlueprintSpawnableComponent))
class AGORA_API UAgoraTimeLevelComp : public UAgoraLevelComponent
{
	GENERATED_BODY()
	
protected:
	
	// If true, the SecondsToLevelUp property will be considered since the start of the game
	// Instead of this actor's begin play
	UPROPERTY(EditDefaultsOnly, Category = "Time Level Component")
	bool bLevelsFromGameTime = true;

	virtual void BeginPlay() override;

	void HandleLevelTimer();
};
