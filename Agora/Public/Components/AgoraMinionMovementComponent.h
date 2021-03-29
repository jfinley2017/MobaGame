// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Components/GASCharacterMovementComponent.h"
#include "AgoraMinionMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraMinionMovementComponent : public UGASCharacterMovementComponent
{
	GENERATED_BODY()
	
	
public:
    
    UAgoraMinionMovementComponent(const FObjectInitializer& ObjectInitializer);

    // UActorComponent
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    // ~UActorComponent 


protected:


};
