// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "AgoraTypes.h"

#include "DamageDealerInterface.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageDealerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Provides functionality for actors which deal damage.
 */
class AGORA_API IDamageDealerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

    /**
     * Alerts the actor that it instigated damage.
     * @Param DamageContext The damage it instigated.
     */
    UFUNCTION()
    virtual void NotifyDamageInstigated(const FDamageContext& DamageContext) = 0;

    /**
     * Retrieves a delegate which can be used to receive notifications for when this actor receives damage.
     */
    virtual FDamageInstigatedSignature& GetDamageInstigatedDelegate() = 0;

};
