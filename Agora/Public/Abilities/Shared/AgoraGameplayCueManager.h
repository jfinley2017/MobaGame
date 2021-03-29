// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "AgoraGameplayCueManager.generated.h"

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()
	
	

protected:

    virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override;

};
