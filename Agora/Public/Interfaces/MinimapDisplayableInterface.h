// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MinimapDisplayableInterface.generated.h"

class UAgoraMapIconComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UMinimapDisplayableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AGORA_API IMinimapDisplayableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

    virtual UAgoraMapIconComponent* GetMapIconComponent() = 0;
  
};
