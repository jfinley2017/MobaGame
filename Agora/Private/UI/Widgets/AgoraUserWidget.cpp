// Fill out your copyright notice in the Description page of Project Settings.

#include "AgoraUserWidget.h"
#include "AgoraUIBlueprintFunctionLibrary.h"


void UAgoraUserWidget::SetObserved(AActor* InObservedActor)
{
    Observed = InObservedActor;
    OnObservedActorChanged(Observed);
}
AActor* UAgoraUserWidget::GetObserved()
{
    return Observed;
}

