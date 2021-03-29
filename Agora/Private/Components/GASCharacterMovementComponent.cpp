// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "GASCharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "AgoraMinionBase.h"

//ALS Structure
/*
There are effectively two places where the ALS resides in this CMC.

The first is in Tick(), where we've got DoEveryFrame() and ManageCharacterRotation(). These two functions manage the calculations and other stuff that has to be happening all the time for the ALS to work.

The second is in OnPawnMovementModeChanged(). Inside, we set ALS_MovementMode based on the CMC's internal EMovementMode flag. The movement mode is integral to the ALS (running, falling, etc).

*/

/*
Note that AgoraCharacterBase's OnMovementModeChanged() play a part as well. OnMovementModeChanged() simply calls into GASCMC->OnPawnMovementModeChanged(). It acts as a proxy for updating the movement mode in the ALS itself.
*/

/*
The remainder of the ALS calculations reside within AgoraAnimInstanceBase.
*/



UGASCharacterMovementComponent::UGASCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bReplicates = true;
}

void UGASCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
    // causes massive LogTick spam
	//GetOwningCharacter()->AddTickPrerequisiteActor(GetOwningCharacter());
	BrakingFrictionFactor = 1.f;
}
float UGASCharacterMovementComponent::GetMaxSpeed() const
{
    AAgoraCharacterBase* Character = CastChecked<AAgoraCharacterBase>(GetOwner(), ECastCheckedType::NullAllowed);
    return Character ? Character->GetMaxSpeed() : Super::GetMaxSpeed();
}

void UGASCharacterMovementComponent::PostLoad()
{
    Super::PostLoad();

    // This has some low level pointer fuckery involved which makes calls to GetMaxSpeed bug out and return the base movement
    // speed value forever. Really odd, haven't looked into it. Still am interested in caching this reference somehow in the future tho
    //OwningGASCharacter = Cast<AAgoraCharacterBase>(GetOwner());
}

//The entirety of the ALS calculations stem from here.
void UGASCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Don't run these for minions.
	if (!GetOwner()->IsA(AAgoraMinionBase::StaticClass()))
	{
		DoEveryFrame();
		//ManageCharacterRotation();
	}
}

void UGASCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//ALS replicated properties
	DOREPLIFETIME_CONDITION(UGASCharacterMovementComponent, MovementInput, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UGASCharacterMovementComponent, CharacterRotation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UGASCharacterMovementComponent, LookingRotation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UGASCharacterMovementComponent, TargetRotation, COND_SkipOwner);
}

void UGASCharacterMovementComponent::SetALS_MovementModeEvent(EALS_MovementMode InALS_MovementMode)
{
	if (InALS_MovementMode != ALS_MovementMode)
	{
		ALS_PrevMovementMode = ALS_MovementMode;
		ALS_MovementMode = InALS_MovementMode;
		OnALSMovementModeChanged();
	}
}

void UGASCharacterMovementComponent::CalculateEssentialVariables()
{
	bIsMoving = !UKismetMathLibrary::EqualEqual_VectorVector(FVector(GetOwningCharacter()->GetVelocity().X, GetOwningCharacter()->GetVelocity().Y, 0.0f), FVector::ZeroVector, 1.0f);
	if (bIsMoving)
	{
		LastVelocityRotation = UKismetMathLibrary::Conv_VectorToRotator(GetOwningCharacter()->GetVelocity());
		Direction = UKismetMathLibrary::NormalizedDeltaRotator(LastVelocityRotation, CharacterRotation).Yaw;
	}
	MovementInput = GetLastInputVector();
	bHasMovementInput = !UKismetMathLibrary::EqualEqual_VectorVector(MovementInput, FVector::ZeroVector, 0.0001f);
	if (bHasMovementInput)
	{
		LastMovementInputRotation = UKismetMathLibrary::Conv_VectorToRotator(MovementInput);
		MovementInputVelocityDifference = UKismetMathLibrary::NormalizedDeltaRotator(LastMovementInputRotation, LastVelocityRotation).Yaw;
	}
	float PrevAimYaw = LookingRotation.Yaw;
	LookingRotation = GetOwningCharacter()->GetControlRotation();
	AimYawRate = (LookingRotation.Yaw - PrevAimYaw) / GetWorld()->GetDeltaSeconds();
	
	if (IsNetworked() && IsLocallyControlled())
	{
		SetLookingRotationAndMovementInput_Server(LookingRotation, MovementInput);
	}
	AimYawDelta = UKismetMathLibrary::NormalizedDeltaRotator(LookingRotation, CharacterRotation).Yaw;
}

void UGASCharacterMovementComponent::OnPawnMovementModeChanged(EMovementMode PrevMovementMode, EMovementMode NewMovementMode, uint8 PrevCustomMode, uint8 NewCustomMove)
{
	//Don't run these for minions.
	if (!GetOwner()->IsA(AAgoraMinionBase::StaticClass()))
	{
		switch (NewMovementMode)
		{
		case EMovementMode::MOVE_Walking:
		case EMovementMode::MOVE_NavWalking:
			SetALS_MovementModeEvent(EALS_MovementMode::Grounded);
			break;
		case EMovementMode::MOVE_Falling:
			SetALS_MovementModeEvent(EALS_MovementMode::Falling);
		}
	}
}

void UGASCharacterMovementComponent::OnALSMovementModeChanged()
{
	if (AnimInstanceAsALSInterface()) IAgoraALSInterface::Execute_ALS_SetALS_MovementMode(AnimInstance(), ALS_MovementMode);
	if (PostProcessInstanceAsALSInterface()) IAgoraALSInterface::Execute_ALS_SetALS_MovementMode(PostProcessInstance(), ALS_MovementMode);
	UpdateALSCharacterMovementSettings();
	if (IsLocallyControlled())
	{
		if (ALS_PrevMovementMode == EALS_MovementMode::Grounded)
		{
			if (bIsMoving)
				JumpRotation = LastVelocityRotation;
			else
				JumpRotation = CharacterRotation;
			RotationOffset = 0.0f;
		}
	}
}

void UGASCharacterMovementComponent::SetCharacterRotation(FRotator InTargetRotation, bool InbInterpRotation, float InInterpSpeed)
{
	TargetRotation = InTargetRotation;
	TargetCharacterRotationDifference = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, CharacterRotation).Yaw;
	if (InbInterpRotation)
	{
		if (InInterpSpeed != 0.0f)
		{
			CharacterRotation = UKismetMathLibrary::RInterpTo(CharacterRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), InInterpSpeed);
		}
	}
	else
	{
		CharacterRotation = TargetRotation;
	}
	GetOwner()->SetActorRotation(CharacterRotation);
	if (IsNetworked())
	{
		SetCharacterRotationEvent_Server(TargetRotation, CharacterRotation);
	}
}

void UGASCharacterMovementComponent::LockCharacterRotationToLookingPoint()
{
	SetCharacterRotation(FRotator(CharacterRotation.Pitch, LookingRotation.Yaw, CharacterRotation.Roll), false, 1.0f);
}

void UGASCharacterMovementComponent::AddCharacterRotation(FRotator AddAmount)
{
	TargetRotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, UKismetMathLibrary::NegateRotator(AddAmount));
	TargetCharacterRotationDifference = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, CharacterRotation).Yaw;
	CharacterRotation = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, UKismetMathLibrary::NegateRotator(AddAmount));
	GetOwner()->SetActorRotation(CharacterRotation);
	if (IsNetworked())
	{
		SetCharacterRotationEvent_Server(TargetRotation, CharacterRotation);
	}
}

FRotator UGASCharacterMovementComponent::LookingDirectionWithOffset(float OffsetInterpSpeed, float NEAngle, float NWAngle, float SEAngle, float SWAngle, float Buffer)
{
	float Value = UKismetMathLibrary::NormalizedDeltaRotator(bHasMovementInput ? LastMovementInputRotation : LastVelocityRotation, LookingRotation).Yaw;

	if (CardinalDirectionAngles(Value, NWAngle, NEAngle, Buffer, ECardinalDirection::North))
	{
		CardinalDirection = ECardinalDirection::North;
	}
	else if (CardinalDirectionAngles(Value, NEAngle, SEAngle, Buffer, ECardinalDirection::East))
	{
		CardinalDirection = ECardinalDirection::East;
	}
	else if (CardinalDirectionAngles(Value, SWAngle, NWAngle, Buffer, ECardinalDirection::West))
	{
		CardinalDirection = ECardinalDirection::West;
	}
	else
	{
		CardinalDirection = ECardinalDirection::South;
	}
	float TargetOffset = 0.0f;
	switch (CardinalDirection)
	{
	case ECardinalDirection::North:
		TargetOffset = Value;
		break;
	case ECardinalDirection::East:
		TargetOffset = Value - 90.0f;
		break;
	case ECardinalDirection::West:
		TargetOffset = Value + 90.0f;
		break;
	case ECardinalDirection::South:
		if (Value > 0.0f)
			TargetOffset = Value - 180.0f;
		else
			TargetOffset = Value + 180.0f;
		break;
	}
	RotationOffset = UKismetMathLibrary::FInterpTo(RotationOffset, TargetOffset, GetWorld()->GetDeltaSeconds(), OffsetInterpSpeed);
	return FRotator(0.0f, LookingRotation.Yaw + RotationOffset, 0.0f);
}

float UGASCharacterMovementComponent::CalculateRotationRate(float SlowSpeed, float SlowSpeedRate, float FastSpeed, float FastSpeedRate)
{
	float Length = FVector(GetOwningCharacter()->GetVelocity().X, GetOwningCharacter()->GetVelocity().Y, 0.0f).Size();
	float ClampedVal = Length > SlowSpeed ? UKismetMathLibrary::MapRangeUnclamped(Length, SlowSpeed, FastSpeed, SlowSpeedRate, FastSpeedRate) : UKismetMathLibrary::MapRangeClamped(Length, 0.0f, SlowSpeed, 0.0f, SlowSpeedRate);
	float ValToReturn = FMath::Clamp(ClampedVal * RotationRateMultiplier, 0.1f, 15.0f);
	if (RotationRateMultiplier != 1.0f)
		RotationRateMultiplier = FMath::Clamp(RotationRateMultiplier + GetWorld()->GetDeltaSeconds(), 0.0f, 1.0f);
	return ValToReturn;
}

void UGASCharacterMovementComponent::UpdateALSCharacterMovementSettings()
{
	MaxWalkSpeed = WalkingSpeed;
	MaxWalkSpeedCrouched = MaxWalkSpeed;
	MaxAcceleration = RunningAcceleration;
	BrakingDecelerationWalking = RunningDeceleration;
	GroundFriction = RunningGroundFriction;
}

void UGASCharacterMovementComponent::CustomAcceleration()
{
	MaxAcceleration = RunningAcceleration * UKismetMathLibrary::MapRangeClamped(FMath::Abs(MovementInputVelocityDifference), 45.0f, 130.0f, 1.0f, 0.2f);
	GroundFriction = RunningGroundFriction * UKismetMathLibrary::MapRangeClamped(FMath::Abs(MovementInputVelocityDifference), 45.0f, 130.0f, 1.0f, 0.4f);
}

void UGASCharacterMovementComponent::PlayerMovementInput(bool InbIsForwardAxis)
{
	if (InbIsForwardAxis)
	{
		GetOwningCharacter()->AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0.0f, GetOwningCharacter()->GetControlRotation().Yaw, 0.0f)), ForwardAxisValue);
	}
	else
	{
		GetOwningCharacter()->AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0.0f, GetOwningCharacter()->GetControlRotation().Yaw, 0.0f)), RightAxisValue);
	}
}

void UGASCharacterMovementComponent::ALS_AddCharacterRotation(FRotator AddAmount)
{
	if (IsLocallyControlled())
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::Delay(this, 0.0f, LatentInfo); //yes, two in a row. it's frame fuckery. not my idea.
		UKismetSystemLibrary::Delay(this, 0.0f, LatentInfo);
		AddCharacterRotation(AddAmount);
	}
}

void UGASCharacterMovementComponent::NetMulticast_PlayNetworkedMontage_Implementation(UAnimMontage* InMontageToPlay, float InPlayRate, float InTimeToStartMontageAt, bool bStopAllMontages)
{
	if (!IsLocallyControlled())
	{
		AnimInstance()->Montage_Play(InMontageToPlay, InPlayRate, EMontagePlayReturnType::MontageLength, InTimeToStartMontageAt, bStopAllMontages);
	}
}

void UGASCharacterMovementComponent::PlayNetworkedMontage_Server_Implementation(UAnimMontage* InMontageToPlay, float InPlayRate, float InTimeToStartMontageAt, bool bStopAllMontages)
{
	NetMulticast_PlayNetworkedMontage(InMontageToPlay, InPlayRate, InTimeToStartMontageAt, bStopAllMontages);
}

bool UGASCharacterMovementComponent::PlayNetworkedMontage_Server_Validate(UAnimMontage* InMontageToPlay, float InPlayRate, float InTimeToStartMontageAt, bool bStopAllMontages) { return true; }

void UGASCharacterMovementComponent::SetCharacterRotationEvent_Server_Implementation(FRotator InTargetRotation, FRotator InCharacterRotation)
{
	TargetRotation = InTargetRotation;
	CharacterRotation = InCharacterRotation;
	GetOwner()->SetActorRotation(CharacterRotation);
}

bool UGASCharacterMovementComponent::SetCharacterRotationEvent_Server_Validate(FRotator InTargetRotation, FRotator InCharacterRotation) { return true; }

void UGASCharacterMovementComponent::SetLookingRotationAndMovementInput_Server_Implementation(FRotator InLookingRotation, FVector InMovementInput)
{
	LookingRotation = InLookingRotation;
	MovementInput = InMovementInput;
}

bool UGASCharacterMovementComponent::SetLookingRotationAndMovementInput_Server_Validate(FRotator InLookingRotation, FVector InMovementInput) { return true; }

bool UGASCharacterMovementComponent::CardinalDirectionAngles(float Value, float Min, float Max, float Buffer, ECardinalDirection InCardinalDirection)
{
	if (InCardinalDirection == CardinalDirection)
	{
		return UKismetMathLibrary::InRange_FloatFloat(Value, Min - Buffer, Max + Buffer);
	}
	return UKismetMathLibrary::InRange_FloatFloat(Value, Min + Buffer, Max - Buffer);
}

UAnimInstance* UGASCharacterMovementComponent::AnimInstance()
{
	return GetOwningCharacter()->GetMesh()->GetAnimInstance();
}

UAnimInstance* UGASCharacterMovementComponent::PostProcessInstance()
{
	return GetOwningCharacter()->GetMesh()->GetPostProcessInstance();
}

TScriptInterface<IAgoraALSInterface> UGASCharacterMovementComponent::AnimInstanceAsALSInterface()
{
	TScriptInterface<IAgoraALSInterface> Interface = AnimInstance();
	return Interface;
}

TScriptInterface<IAgoraALSInterface> UGASCharacterMovementComponent::PostProcessInstanceAsALSInterface()
{
	TScriptInterface<IAgoraALSInterface> Interface = PostProcessInstance();
	return Interface;
}

bool UGASCharacterMovementComponent::IsNetworked()
{
	return !UKismetSystemLibrary::IsStandalone(this);
}

bool UGASCharacterMovementComponent::IsLocallyControlled()
{
	return GetOwningCharacter()->IsLocallyControlled();
}

void UGASCharacterMovementComponent::DoEveryFrame()
{
	CalculateEssentialVariables();
	if (ALS_MovementMode == EALS_MovementMode::Grounded) 
	{
		DoWhileGrounded();
	}
}

void UGASCharacterMovementComponent::DoWhileGrounded()
{
	CustomAcceleration();
}

void UGASCharacterMovementComponent::ManageCharacterRotation()
{
	if (IsLocallyControlled())
	{
		switch (ALS_MovementMode)
		{
		case EALS_MovementMode::Grounded:
			Grounded();
			break;
		case EALS_MovementMode::Falling:
			Falling();
			break;
		}
	}
}

void UGASCharacterMovementComponent::Grounded()
{
	if (bIsMoving)
	{
		FRotator Rotation = LookingDirectionWithOffset(5.0f, 60.0f, -60.0f, 120.0f, -120.0f, 5.0f);
		float _RotationRate = CalculateRotationRate(165.0f, 10.0f, 375.0f, 15.0f);
		SetCharacterRotation(Rotation, true, _RotationRate);
	}
}

void UGASCharacterMovementComponent::Falling()
{
	JumpRotation = LookingRotation;
	SetCharacterRotation(FRotator(0.0f, JumpRotation.Yaw, 0.0f), true, 10.0f);
}