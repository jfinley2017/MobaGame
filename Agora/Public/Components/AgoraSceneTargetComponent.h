// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AgoraSceneTargetComponent.generated.h"

/************************************************************************/
/*                        SceneTargetComponent                          */
/*                                                                      */
/* Component used to designate where homing targets should travel to    */
/* on a unit                                                            */
/************************************************************************/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGORA_API UAgoraSceneTargetComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAgoraSceneTargetComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	
};
