// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AgoraUserWidget.generated.h"

/**
 * Wraps normal UUserWidget with the ability to observe an actor, actors which are observed function similarily to the model in a typical MVC setup.
 */
UCLASS()
class AGORA_API UAgoraUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "UIUserWidget")
    virtual void SetObserved(AActor* InObservedActor);

    UFUNCTION(BlueprintPure, Category = "UIUserWidget")
    AActor* GetObserved();

protected:

    /** Blueprint hook for when our observed actor has changed */
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UIUserWidget")
    void OnObservedActorChanged(AActor* NewObservedActor);

    UPROPERTY(BlueprintReadWrite, Category = "UIUserWidget")
    AActor* Observed = nullptr;

  
};
