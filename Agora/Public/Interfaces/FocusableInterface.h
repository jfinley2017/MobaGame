// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FocusableInterface.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusedActorSignature, AActor*, Actor);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFocusableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AGORA_API IFocusableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Focusable")
    void NotifyFocused(AActor* FocusingActor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Focusable")
    void NotifyEndFocused(AActor* EndFocusingActor);

    virtual FOnFocusedActorSignature& GetStartFocusedDelegate() = 0;
    virtual FOnFocusedActorSignature& GetEndFocusedDelegate() = 0;
    virtual FOnFocusedActorSignature& GetFocusChangedDelegate() = 0;

};
