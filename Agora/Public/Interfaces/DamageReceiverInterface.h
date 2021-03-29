// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "AgoraTypes.h"

#include "DamageReceiverInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Provides functionality for actors which receive damage.
 */
class AGORA_API IDamageReceiverInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

    
    /**
     * Alerts the actor that it received damage.
     * @Param CurrentHP  the actor's new HP
     * @Param MaxHP the actor's MaxHP
     * @Param DamageContext context surrounding the damage taken
     */
    UFUNCTION()
    virtual void NotifyDamageReceived(float CurrentHP, float MaxHP, const FDamageContext& DamageContext) = 0;

    /**
     * Alerts the actor that it died. 
     * @Param DamageContext Damage that killed the actor
     */
    UFUNCTION()
    virtual void NotifyDied(const FDamageContext& DamageContext) = 0;

    /**
     * Returns whether or not the actor implementing this interface is dead.
     * @Return true for dead, false for alive
     */
    UFUNCTION()
    virtual bool IsDead() const = 0;

    /**
     * Retrieves a delegate which can be used to receive notifications for when this actor receives damage.
     */
    virtual FDamageReceivedSignature& GetDamageReceivedDelegate() = 0;

    /**
     * Retrieves a delegate which can be used to receive notifications for when this actor dies.
     */
    virtual FDiedSignature& GetDeathDelegate() = 0;

};
