// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/AgoraUserWidget.h"
#include "AgoraMinimap.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraMinimap : public UAgoraUserWidget
{
	GENERATED_BODY()
	
public:

 
protected:
    
    // UUserWidget interface
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	// ~UUserWidget interface

    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraMinimap")
    void UpdateIcons(const FGeometry& MyGeometry, float InDeltaTime);
   
};
