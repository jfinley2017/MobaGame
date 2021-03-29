// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VisionInterface.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVisibilityToggledSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVisionLevelChangedSignature, AActor*, ChangedActor, uint8, NewVisionLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHiddenLevelChangedSignature, AActor*, ChangedActor, uint8, NewHiddenLevel);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UVisionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AGORA_API IVisionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

    UFUNCTION()
    virtual void NotifyRevealed(AActor* Revealer) = 0;

    UFUNCTION()
    virtual void NotifyRevealedEnd(AActor* StoppedRevealer) = 0;

    UFUNCTION()
    virtual bool IsVisibleFor(AActor* ViewingActor) const = 0;

    UFUNCTION()
    virtual uint8 GetVisionLevel() const = 0;

    UFUNCTION()
    virtual uint8 GetHiddenLevel() const = 0;

    UFUNCTION()
    virtual void GetViewLocation(FVector& OutViewLocation) const = 0;

    virtual FOnVisibilityToggledSignature& GetBecameVisibleDelegate() = 0;
    virtual FOnVisibilityToggledSignature& GetBecameHiddenDelegate() = 0;
    virtual FOnVisionLevelChangedSignature& GetVisionLevelChangedDelegate() = 0;
    virtual FOnHiddenLevelChangedSignature& GetHiddenLevelChangedDelegate() = 0;

};
