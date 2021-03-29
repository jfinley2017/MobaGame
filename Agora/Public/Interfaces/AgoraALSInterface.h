// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "AgoraTypes.h"
#include "UObject/Interface.h"
#include "AgoraALSInterface.generated.h"

/*
AgoraALSInterface

This guy is just a C++ rewrite of the ALS BP Interface.
It is implemented by both AgoraCharacterBase and AgoraAnimInstanceBase.
The enums and such are declared in AgoraTypes.h

Functions in this interface are always prefixed with "ALS_"
*/

class UCameraShake;

UINTERFACE(BlueprintType)
class UAgoraALSInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AGORA_API IAgoraALSInterface
{
	GENERATED_BODY()

public:

	//ALS Values
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
    void ALS_SetALS_Gait(const EALS_Gait ALS_Gait);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SetALS_Stance(const EALS_Stance ALS_Stance);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SetALS_RotationMode(const EALS_RotationMode ALS_RotationMode);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SetALS_MovementMode(const EALS_MovementMode ALS_MovementMode);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SetALS_ViewMode(const EALS_ViewMode ALS_ViewMode);


	//ALS Character Movement Settings
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SetWalkingSpeed(float WalkingSpeed);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SetRunningSpeed(float RunningSpeed);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SetSprintingSpeed(float SprintingSpeed);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SetCrouchingSpeed(float CrouchingSpeed);


	//ALS Anim Notify
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_AnimNotify_Pivot(FAgoraALS_PivotParams PivotParams);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_AnimNotify_TurningInPlace(UAnimMontage* TurnInPlaceMontage, bool bShouldTurnInPlace, bool bTurningInPlace, bool bTurningRight);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_AnimNotify_IdleEntryState(const EIdleEntryState IdleEntryState);

	//General Interface functions
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_Play_GetUp_Anim(bool bFaceDown);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SavePoseSnapshot(FName PoseName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_AddCharacterRotation(FRotator AddAmount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_CameraShake(TSubclassOf<UCameraShake> ShakeClass, float Scale);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALSInterface")
	void ALS_SetIsFiring(bool bInIsFiring);
};
