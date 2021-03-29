// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AgoraTypes.h"
#include "AgoraALSInterface.h"
#include "AgoraAnimInstanceBase.generated.h"

class AAgoraCharacterBase;
class UGASCharacterMovementComponent;
class UCapsuleComponent;
class UAnimMontage;
class UAnimSequenceBase;
class UCurveFloat;

/**
 * Base AnimBP containing all skeleton-agnostic ALS code
 */
UCLASS(Blueprintable, BlueprintType)
class AGORA_API UAgoraAnimInstanceBase : public UAnimInstance, public IAgoraALSInterface
{
	GENERATED_BODY()


	//"macros"
	bool IsNetworked();

	bool IsLocallyControlled();

protected:
	//references
	UPROPERTY(BlueprintReadWrite, Category = "References")
	AAgoraCharacterBase* Character = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "References")
	UGASCharacterMovementComponent* CharacterMovement = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "References")
	UCapsuleComponent* CapsuleComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimReferences|Additive")
	UAnimSequenceBase* AdditiveLand = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimReferences|TurnInPlace")
	FAgoraTurnAnims N_Turn_90;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimReferences|TurnInPlace")
	FAgoraTurnAnims N_Turn_180;

	//for anim graph
	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	FVector Velocity;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	FVector2D AimOffset;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	FVector2D LeanGrounded;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	FVector2D FeetPosition;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	EMovementDirection MovementDirection = EMovementDirection::Forwards;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	EIdleEntryState IdleEntryState = EIdleEntryState::N_Idle;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	EALS_MovementMode ALS_PrevMovementMode = EALS_MovementMode::Grounded;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	EALS_MovementMode ALS_MovementMode = EALS_MovementMode::Grounded;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	EALS_Gait ALS_Gait = EALS_Gait::Walking;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	EALS_Stance ALS_Stance = EALS_Stance::Standing;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	EALS_ViewMode ALS_ViewMode = EALS_ViewMode::ThirdPerson;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	float Direction = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	float GaitValue = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	float N_PlayRate = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	float StartPosition = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	float TargetCharacterRotationDifference = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	float MovementInputVelocityDifference = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	bool bShouldTurnInPlace = false;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	bool bTurnInPlaceEnabled = true;

	UPROPERTY(BlueprintReadWrite, Category = "ForAnimGraph")
	bool bIsFiring = false;

	//turn in place system

	UPROPERTY(BlueprintReadWrite, Category = "TurnInPlaceSystem")
	bool bTurningInPlace = false;

	UPROPERTY(BlueprintReadWrite, Category = "TurnInPlaceSystem")
	bool bTurningRight = false;

	UPROPERTY(BlueprintReadWrite, Category = "TurnInPlaceSystem")
	float TurnInPlaceDelayCount = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "TurnInPlaceSystem")
	UAnimMontage* ActiveTurnInPlaceMontage = nullptr;

	//pivot system

	UPROPERTY(BlueprintReadWrite, Category = "PivotSystem")
	FAgoraALS_PivotParams PivotParams;

	//leaning calculations

	UPROPERTY(BlueprintReadWrite, Category = "LeaningCalculations")
	FRotator PreviousVelocityRotation;

	UPROPERTY(BlueprintReadWrite, Category = "TurnInPlaceSystem")
	float VelocityDifference = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "TurnInPlaceSystem")
	float PreviousSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "TurnInPlaceSystem")
	float AccelerationDifference = 0.0f;

	//Rotators

	UPROPERTY(BlueprintReadWrite, Category = "Rotators")
	FRotator CharacterRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Rotators")
	FRotator LastVelocityRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Rotators")
	FRotator LastMovementInputRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Rotators")
	FRotator LookingRotation;

	//Floats

	UPROPERTY(BlueprintReadWrite, Category = "Floats")
	float AimYawDelta = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Floats")
	float AimYawRate = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Floats")
	float WalkingSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Floats")
	float RunningSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Floats")
	float SprintingSpeed = 0.0f;

	//Enums
	UPROPERTY(BlueprintReadWrite, Category = "Enums")
	ELocomotionState ActiveLocomotionState = ELocomotionState::None;

	//Functions

	//collapsed graph equivalents
	void DoEveryFrame(); 

	void AllTheTime();

	void WhileFalling();

	void WhileGrounded();

	void WhileMoving();

	void TurnInPlace();

	void InNotMovingState();

	void InMovingState();

	void InPivotState();

	//actual functions

	//Initialize
	void SetReferences();

	void SetDefaultValues();

	//All the time
	void CalculateAimOffset();

	void GetVariablesFromBaseCharacter();

	//On changed
	void OnALS_MovementModeChanged();

	//On ground
	void CalculateMovementDirection(float DirectionThresholdMin, float DirectionThresholdMax, float Buffer);

	void CalculateGroundedLeaningValues();

	void CalculateStartPosition();

	void CalculateGaitValue();

	void CalculatePlayRates(float WalkAnimSpeed, float RunAnimSpeed, float SprintAnimSpeed);
	
	//Turn in place system
	void TurnInPlaceDelay(float MaxCameraSpeed, float AimYawLimit1, float DelayTime1, float PlayRate1, FAgoraTurnAnims TurnAnims1, float AimYawLimit2, float DelayTime2, float PlayRate2, FAgoraTurnAnims TurnAnims2);

	//Notify Events will be implemented in BP (just easier that way.)

	//Custom Events
	void PlayMontageNetworked(UAnimMontage* MontageToPlay, float InPlayRate, float InTimeToStartMontageAt, bool bStopAllMontages);

	//ALS Interface events
	virtual void ALS_SetALS_MovementMode_Implementation(EALS_MovementMode InALS_MovementMode) override;

	virtual void ALS_AddCharacterRotation_Implementation(FRotator AddAmount);

	virtual void ALS_SetWalkingSpeed_Implementation(float InWalkingSpeed) override;

	virtual void ALS_SetRunningSpeed_Implementation(float InRunningSpeed) override;

	virtual void ALS_SetSprintingSpeed_Implementation(float InSprintingSpeed) override;

	virtual void ALS_SavePoseSnapshot_Implementation(FName InPoseName) override;

	virtual void ALS_SetIsFiring_Implementation(bool bInIsFiring) override;

	virtual void ALS_AnimNotify_Pivot_Implementation(FAgoraALS_PivotParams InPivotParams) override;

	virtual void ALS_AnimNotify_IdleEntryState_Implementation(const EIdleEntryState InIdleEntryState) override;

	virtual void ALS_AnimNotify_TurningInPlace_Implementation(UAnimMontage* TurnInPlaceMontage, bool bInShouldTurnInPlace, bool bInTurningInPlace, bool bInTurningRight) override;

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaTimeX) override;
};
