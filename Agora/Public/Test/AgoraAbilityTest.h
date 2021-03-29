// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "AgoraTypes.h"
#include "AgoraAbilityTest.generated.h"

class AAgoraHeroBase;

/**
 * 
 */
UCLASS()
class AGORA_API AAgoraAbilityTest : public AFunctionalTest
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "AbilityTest")
	virtual void UseAbility();

	virtual void AcquireAbility();

protected:
	UFUNCTION(BlueprintCallable, Category = "AbilityTest")
	void Quit();

	virtual void GatherRelevantActors(TArray<AActor*>& OutActors) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityTest|Initialization")
	bool bIsTargeted = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityTest|Initialization")
	AAgoraHeroBase* Hero = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityTest|Initialization")
	EAbilityInput AbilitySlot = EAbilityInput::AbilityPrimary;

	// The hero will try to use the ability on this target if the hero has its controller set to AI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilityTest|Initialization")
	AActor* AbilityTarget = nullptr;

};
