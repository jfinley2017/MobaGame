// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "Interfaces/TeamInterface.h"
#include "AgoraTeamActor.generated.h"

/**
 * Actor with a team - currently only intended for the team camera view targets
 */
UCLASS()
class AGORA_API AAgoraTeamActor : public ACameraActor, public ITeamInterface
{
	GENERATED_BODY()
public:
	// Begin TeamInterface
	FOnTeamChangedSignature TeamChanged;

	UFUNCTION(BlueprintCallable, Category = "TeamInterface")
	ETeam GetTeam() const override;
	UFUNCTION(BlueprintCallable, Category = "TeamInterface")
	void SetTeam(ETeam NewTeam) override;

	virtual FOnTeamChangedSignature& GetTeamChangedDelegate() override;
	// ~TeamInterface

protected:
	ETeam Team;
};
