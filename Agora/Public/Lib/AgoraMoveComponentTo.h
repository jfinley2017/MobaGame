// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AgoraMoveComponentTo.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAgoraMoveComponentToDelegate);

class USceneComponent;

/**
 *  Works just like the built-in MoveComponentTo, but with async support. Makes it more flexible to use in front of a WaitTargetData.
 *  This guy is primarily meant for moving the player's camera around, although it technically will work with any USceneComponent*.

 *  Always prefer this guy over using the built-in UE MoveComponentTo.

 *  NOTA BENE: In the context of a GA, Do NOT bother to use this on WaitTargetDataCancelled. Reason is that if you plan to end the ability on target cancel, 
 *  assuming you try to use this asynchronously, the ability will end before this guy is done moving the camera, and thus the camera 
 *  won't actually get to move. If you elect to EndAbility() on Completed(), then it'll work, but then there's no point in using this node; you might as well
 *  use AgoraGameplayAbility::ResetCamera(), which is latent/synchronous. It's FAR cleaner.

 *  To summarize: if you need to be asynchronous in a GA, use this. Else, use the AgoraGameplayAbility camera movement functions and reduce spaghetti.
 */
UCLASS()
class AGORA_API UAgoraMoveComponentTo : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
private:
	UObject* WorldContextObject;
	USceneComponent* Component;
	FVector TargetRelativeLocation;
	FRotator TargetRelativeRotation;
	bool bEaseIn;
	bool bEaseOut;
	float OverTime;
	FLatentActionInfo LatentInfo;

	UFUNCTION()
		void _Completed();

public:
	UPROPERTY(BlueprintAssignable)
		FAgoraMoveComponentToDelegate Completed;

	UFUNCTION(BlueprintCallable, meta = (LatentInfo = "LatentInfo", BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "AgoraAsyncLibrary")
		static UAgoraMoveComponentTo* MoveComponentTo(UObject* WorldContextObject, USceneComponent* Component, FVector TargetRelativeLocation, FRotator TargetRelativeRotation, bool bEaseIn, bool bEaseOut, float OverTime, FLatentActionInfo LatentInfo);

	virtual void Activate() override;
};
