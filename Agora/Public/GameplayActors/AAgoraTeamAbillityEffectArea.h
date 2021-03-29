// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameplayActors/AgoraAbilityEffectArea.h"
#include "AAgoraTeamAbillityEffectArea.generated.h"

/**
 * Meant to be placed in the world. Mainly for the fountain
 */
UCLASS()
class AGORA_API AAgoraTeamAbillityEffectArea : public AAgoraAbilityEffectArea, public ITeamInterface
{
	GENERATED_BODY()
	
public:
	AAgoraTeamAbillityEffectArea(const FObjectInitializer& ObjectInitializer);

	// TeamInterface interface
	UFUNCTION(BlueprintCallable, Category = "TeamInterface")
		ETeam GetTeam() const override;
	UFUNCTION(BlueprintCallable, Category = "TeamInterface")
		void SetTeam(ETeam NewTeam) override;

	FOnTeamChangedSignature OnTeamChanged;
	virtual FOnTeamChangedSignature& GetTeamChangedDelegate() override;
	// ~TeamInterface interface

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "TeamInterface")
	ETeam Team = ETeam::Invalid;

	TSubclassOf<UGameplayEffect> EffectClass;
};
