// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraAnimInstanceBase.h"

#include "AgoraCharacterBase.h"
#include "GASCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Curves/CurveFloat.h"

bool UAgoraAnimInstanceBase::IsNetworked()
{
	return !UKismetSystemLibrary::IsStandalone(this);
}

bool UAgoraAnimInstanceBase::IsLocallyControlled()
{
	return TryGetPawnOwner()->IsLocallyControlled();
}

void UAgoraAnimInstanceBase::DoEveryFrame()
{
	AllTheTime();
	switch (ALS_MovementMode)
	{
	case EALS_MovementMode::Grounded:
	{
		WhileGrounded();
		switch (ActiveLocomotionState)
		{
		case ELocomotionState::NotMoving:
			InNotMovingState();
			break;
		case ELocomotionState::Moving:
			InMovingState();
			break;
		case ELocomotionState::Pivot:
			InPivotState();
			break;
		}
		break;
	}
	case EALS_MovementMode::Falling:
		WhileFalling();
		break;
	}
}

void UAgoraAnimInstanceBase::AllTheTime()
{
	CalculateAimOffset();
}

void UAgoraAnimInstanceBase::WhileFalling()
{
	Speed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
}

void UAgoraAnimInstanceBase::WhileGrounded()
{
	Speed = Velocity.Size();
	if (bIsMoving) WhileMoving();
	else TurnInPlace();
}

void UAgoraAnimInstanceBase::WhileMoving()
{
	CalculateGaitValue();
	CalculatePlayRates(WalkingSpeed, RunningSpeed, SprintingSpeed);
	CalculateMovementDirection(-90.0f, 90.0f, 5.0f);
}

void UAgoraAnimInstanceBase::TurnInPlace()
{
	if (IsLocallyControlled())
	{
		if (!Character->IsPlayingRootMotion())
		{
			if (!bTurningInPlace) TurnInPlaceDelay(250.0f, 60.0f, 0.5f, 1.5f, N_Turn_90, 130.0f, 0.0f, 1.25f, N_Turn_180);
		}
	}
}

void UAgoraAnimInstanceBase::InNotMovingState()
{
	if (bIsMoving) CalculateStartPosition();
}

void UAgoraAnimInstanceBase::InMovingState()
{
	CalculateGroundedLeaningValues();
}

void UAgoraAnimInstanceBase::InPivotState()
{
	bool NearlyEqual = UKismetMathLibrary::NearlyEqual_FloatFloat(Direction, PivotParams.PivotDirection, 45.0f);
	MovementDirection = NearlyEqual ? PivotParams.InterruptedMovementDirection : PivotParams.CompletedMovementDirection;
	StartPosition = NearlyEqual ? PivotParams.InterruptedStartTime : PivotParams.CompletedStartTime;
}

void UAgoraAnimInstanceBase::SetReferences()
{
	CharacterMovement = Character->GetGASCMC();
	CapsuleComponent = Character->GetCapsuleComponent();
}

void UAgoraAnimInstanceBase::SetDefaultValues()
{
	IAgoraALSInterface::Execute_ALS_SetALS_MovementMode(this, CharacterMovement->ALS_MovementMode);
	IAgoraALSInterface::Execute_ALS_SetWalkingSpeed(this, CharacterMovement->WalkingSpeed);
	IAgoraALSInterface::Execute_ALS_SetRunningSpeed(this, CharacterMovement->RunningSpeed);
	IAgoraALSInterface::Execute_ALS_SetSprintingSpeed(this, CharacterMovement->SprintingSpeed);

	IdleEntryState = EIdleEntryState::N_Idle;
}

void UAgoraAnimInstanceBase::CalculateAimOffset()
{
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(LookingRotation, CharacterRotation);
	AimOffset = UKismetMathLibrary::Vector2DInterpTo(AimOffset,
		FVector2D(Delta.Yaw, Delta.Pitch), GetWorld()->GetDeltaSeconds(),
		UKismetMathLibrary::MapRangeClamped(FMath::Abs(Delta.Yaw - AimOffset.X), 1.0f, 180.0f, 30.0f, 5.0f));
}

void UAgoraAnimInstanceBase::GetVariablesFromBaseCharacter()
{
	Velocity = Character->GetVelocity();
	CharacterRotation = CharacterMovement->CharacterRotation;
	LastVelocityRotation = CharacterMovement->LastVelocityRotation;
	LastMovementInputRotation = CharacterMovement->LastMovementInputRotation;
	LookingRotation = CharacterMovement->LookingRotation;
	Direction = CharacterMovement->Direction;
	TargetCharacterRotationDifference = CharacterMovement->TargetCharacterRotationDifference;
	MovementInputVelocityDifference = CharacterMovement->MovementInputVelocityDifference;
	AimYawDelta = CharacterMovement->AimYawDelta;
	AimYawRate = CharacterMovement->AimYawRate;
	bIsMoving = CharacterMovement->bIsMoving;
	bHasMovementInput = CharacterMovement->bHasMovementInput;
	bTurnInPlaceEnabled = CharacterMovement->bTurnInPlaceEnabled;
}

void UAgoraAnimInstanceBase::OnALS_MovementModeChanged()
{
	if (ALS_MovementMode == EALS_MovementMode::Falling)
	{
		N_PlayRate = 0.0f;
		bShouldTurnInPlace = false;
	}
}

void UAgoraAnimInstanceBase::CalculateMovementDirection(float DirectionThresholdMin, float DirectionThresholdMax, float Buffer)
{
	if (MovementDirection == EMovementDirection::Forwards)
	{
		if (UKismetMathLibrary::InRange_FloatFloat(Direction, DirectionThresholdMin - Buffer, DirectionThresholdMax + Buffer))
		{
			MovementDirection = EMovementDirection::Forwards;
		}
		else
		{
			MovementDirection = EMovementDirection::Backwards;
		}
	}
	else
	{
		if (UKismetMathLibrary::InRange_FloatFloat(Direction, DirectionThresholdMin + Buffer, DirectionThresholdMax - Buffer))
		{
			MovementDirection = EMovementDirection::Forwards;
		}
		else
		{
			MovementDirection = EMovementDirection::Backwards;
		}
	}
}

void UAgoraAnimInstanceBase::CalculateGroundedLeaningValues()
{
	VelocityDifference = UKismetMathLibrary::NormalizedDeltaRotator(LastVelocityRotation, PreviousVelocityRotation).Yaw / GetWorld()->GetDeltaSeconds();
	PreviousVelocityRotation = LastVelocityRotation;
	float LeanRotation = UKismetMathLibrary::MapRangeClamped(VelocityDifference, -200.0f, 200.0f, -1.0f, 1.0f) * UKismetMathLibrary::MapRangeClamped(Speed, WalkingSpeed, RunningSpeed, 0.0f, 1.0f);

	AccelerationDifference = (Speed - PreviousSpeed) / GetWorld()->GetDeltaSeconds();
	PreviousSpeed = Speed;
	float SelectedFloat;
	if (AccelerationDifference > 0.0f)
	{
		SelectedFloat = UKismetMathLibrary::MapRangeClamped(FMath::Abs(AccelerationDifference), 0.0f, CharacterMovement->GetMaxAcceleration(), 0.0f, 1.0f);
	}
	else
	{
		SelectedFloat = UKismetMathLibrary::MapRangeClamped(FMath::Abs(AccelerationDifference), 0.0f, CharacterMovement->BrakingDecelerationWalking, 0.0f, -1.0f);
	}
	float LeanAcceleration = SelectedFloat * UKismetMathLibrary::MapRangeClamped(Speed, WalkingSpeed, RunningSpeed, 0.0f, 1.0f);
	LeanGrounded.X = UKismetMathLibrary::RotateAngleAxis(FVector(LeanRotation, LeanAcceleration, 0.0f), Direction, FVector(0.0f, 0.0f, -1.0f)).X;
	LeanGrounded.Y = UKismetMathLibrary::RotateAngleAxis(FVector(LeanRotation, LeanAcceleration, 0.0f), Direction, FVector(0.0f, 0.0f, -1.0f)).Y;
}

void UAgoraAnimInstanceBase::CalculateStartPosition()
{
	if (Direction > 0.0f) StartPosition = 0.3f;
	else StartPosition = 0.867f;
}

void UAgoraAnimInstanceBase::CalculateGaitValue()
{
	if (Speed > RunningSpeed)
	{
		GaitValue = UKismetMathLibrary::MapRangeClamped(Speed, RunningSpeed, SprintingSpeed, 2.0f, 3.0f);
	}
	else
	{
		if (Speed > WalkingSpeed)
		{
			GaitValue = UKismetMathLibrary::MapRangeClamped(Speed, WalkingSpeed, RunningSpeed, 1.0f, 2.0f);
		}
		else
		{
			GaitValue = UKismetMathLibrary::MapRangeClamped(Speed, 0.0f, WalkingSpeed, 0.0f, 1.0f);
		}
	}
}

void UAgoraAnimInstanceBase::CalculatePlayRates(float WalkAnimSpeed, float RunAnimSpeed, float SprintAnimSpeed)
{
	N_PlayRate = (GaitValue > 2.0 ? UKismetMathLibrary::MapRangeClamped(GaitValue, 2.0f, 3.0f, UKismetMathLibrary::MapRangeUnclamped(Speed, 0.0f, RunAnimSpeed, 0.0f, 1.0f), UKismetMathLibrary::MapRangeUnclamped(Speed, 0.0f, SprintAnimSpeed, 0.0f, 1.0f))
		: UKismetMathLibrary::MapRangeClamped(GaitValue, 1.0f, 2.0f, UKismetMathLibrary::MapRangeUnclamped(Speed, 0.0f, WalkAnimSpeed, 0.0f, 1.0f), UKismetMathLibrary::MapRangeUnclamped(Speed, 0.0f, RunAnimSpeed, 0.0f, 1.0f))) / CapsuleComponent->K2_GetComponentScale().Z;
}

void UAgoraAnimInstanceBase::TurnInPlaceDelay(float MaxCameraSpeed, float AimYawLimit1, float DelayTime1, float PlayRate1, FAgoraTurnAnims TurnAnims1, float AimYawLimit2, float DelayTime2, float PlayRate2, FAgoraTurnAnims TurnAnims2)
{
	if (bTurnInPlaceEnabled)
	{
		if (FMath::Abs(AimYawRate) < MaxCameraSpeed && FMath::Abs(AimYawDelta) > AimYawLimit1)
		{
			TurnInPlaceDelayCount = TurnInPlaceDelayCount + GetWorld()->GetDeltaSeconds();
			bShouldTurnInPlace = TurnInPlaceDelayCount > UKismetMathLibrary::MapRangeClamped(FMath::Abs(AimYawDelta), AimYawLimit1, AimYawLimit2, DelayTime1, DelayTime2);
			UAnimMontage* Montage;
			if (FMath::Abs(AimYawDelta) >= AimYawLimit2)
			{
				if (AimYawDelta > 0.0f) Montage = TurnAnims2.Turn_R_Anim;
				else Montage = TurnAnims2.Turn_L_Anim;
			}
			else
			{
				if (AimYawDelta > 0.0f) Montage = TurnAnims1.Turn_R_Anim;
				else Montage = TurnAnims1.Turn_L_Anim;
			}
			if (bShouldTurnInPlace && !Montage_IsPlaying(Montage))
			{
				PlayMontageNetworked(Montage, FMath::Abs(AimYawDelta) >= AimYawLimit2 ? PlayRate2 : PlayRate1, 0.0f, true);
			}
		}
		else
		{
			TurnInPlaceDelayCount = 0.0f;
		}
	}
}

void UAgoraAnimInstanceBase::PlayMontageNetworked(UAnimMontage* MontageToPlay, float InPlayRate, float InTimeToStartMontageAt, bool bStopAllMontages)
{
	Montage_Play(MontageToPlay, InPlayRate, EMontagePlayReturnType::MontageLength, InTimeToStartMontageAt, bStopAllMontages);
	if (IsNetworked()) CharacterMovement->PlayNetworkedMontage_Server(MontageToPlay, InPlayRate, InTimeToStartMontageAt, bStopAllMontages);
}

void UAgoraAnimInstanceBase::ALS_SetALS_MovementMode_Implementation(EALS_MovementMode InALS_MovementMode)
{
	ALS_PrevMovementMode = ALS_MovementMode;
	ALS_MovementMode = InALS_MovementMode;
	OnALS_MovementModeChanged();
}

void UAgoraAnimInstanceBase::ALS_AddCharacterRotation_Implementation(FRotator AddAmount)
{
	CharacterMovement->ALS_AddCharacterRotation(AddAmount);
}

void UAgoraAnimInstanceBase::ALS_SetWalkingSpeed_Implementation(float InWalkingSpeed)
{
	WalkingSpeed = InWalkingSpeed;
}

void UAgoraAnimInstanceBase::ALS_SetRunningSpeed_Implementation(float InRunningSpeed)
{
	RunningSpeed = InRunningSpeed;
}

void UAgoraAnimInstanceBase::ALS_SetSprintingSpeed_Implementation(float InSprintingSpeed)
{
	SprintingSpeed = InSprintingSpeed;
}

void UAgoraAnimInstanceBase::ALS_SavePoseSnapshot_Implementation(FName InPoseName)
{
	SavePoseSnapshot(InPoseName);
}

void UAgoraAnimInstanceBase::ALS_SetIsFiring_Implementation(bool bInIsFiring)
{
	bIsFiring = bInIsFiring;
}

void UAgoraAnimInstanceBase::ALS_AnimNotify_Pivot_Implementation(FAgoraALS_PivotParams InPivotParams)
{
	PivotParams = InPivotParams;
}

void UAgoraAnimInstanceBase::ALS_AnimNotify_IdleEntryState_Implementation(const EIdleEntryState InIdleEntryState)
{
	IdleEntryState = InIdleEntryState;
}

void UAgoraAnimInstanceBase::ALS_AnimNotify_TurningInPlace_Implementation(UAnimMontage* TurnInPlaceMontage, bool bInShouldTurnInPlace, bool bInTurningInPlace, bool bInTurningRight)
{
	ActiveTurnInPlaceMontage = TurnInPlaceMontage;
	bShouldTurnInPlace = bInShouldTurnInPlace;
	bTurningInPlace = bInTurningInPlace;
	bTurningRight = bInTurningRight;
}

void UAgoraAnimInstanceBase::NativeInitializeAnimation()
{
	if (UKismetSystemLibrary::IsValid(TryGetPawnOwner()))
	{
		Character = Cast<AAgoraCharacterBase>(TryGetPawnOwner());
		SetReferences();
		SetDefaultValues();
	}
}

void UAgoraAnimInstanceBase::NativeUpdateAnimation(float DeltaTimeX)
{
	if (DeltaTimeX != 0.0f && UKismetSystemLibrary::IsValid(Character))
	{
		GetVariablesFromBaseCharacter();
		DoEveryFrame();
	}
}
