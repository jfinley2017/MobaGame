// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AgoraALSInterface.h"
#include "AgoraCharacterBase.h"
#include "AgoraTypes.h"

#include "GASCharacterMovementComponent.generated.h"

class UCameraShake;
class UAnimInstance;
class UAnimMontage;
class UCurveFloat;
class UTimelineComponent;
class USpringArmComponent;

/**
 * Primarily contains ALS functionality.
 */
UCLASS()
class AGORA_API UGASCharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    
    UGASCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

    // UCharacterMovementComponent
    virtual float GetMaxSpeed() const override;
    virtual void PostLoad() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool IsSupportedForNetworking() const override { return true; }
    // ~UCharacterMovementComponent


	AAgoraCharacterBase* GetOwningCharacter() { return Cast<AAgoraCharacterBase>(GetOwner()); }

	//ALS RPCs

	//called every frame
	UFUNCTION(server, Unreliable, WithValidation, Category = "ALSCharacter")
	void SetLookingRotationAndMovementInput_Server(FRotator InLookingRotation, FVector InMovementInput);

	//Called when character is being rotated
	UFUNCTION(server, Unreliable, WithValidation, Category = "ALSCharacter")
	void SetCharacterRotationEvent_Server(FRotator InTargetRotation, FRotator InCharacterRotation);

	//called when turning in place
	UFUNCTION(server, Reliable, WithValidation, Category = "ALSCharacter")
	void PlayNetworkedMontage_Server(UAnimMontage* InMontageToPlay, float InPlayRate, float InTimeToStartMontageAt, bool bStopAllMontages);

	UFUNCTION(NetMulticast, Unreliable, Category = "ALSCharacter")
	void NetMulticast_PlayNetworkedMontage(UAnimMontage* InMontageToPlay, float InPlayRate, float InTimeToStartMontageAt, bool bStopAllMontages);

	//ALS Base Character Events

	void SetALS_MovementModeEvent(EALS_MovementMode InALS_MovementMode);

	// ALS Base Character Functions

	//Essential
	void CalculateEssentialVariables();

	//OnChanged

	void OnPawnMovementModeChanged(EMovementMode PrevMovementMode, EMovementMode NewMovementMode, uint8 PrevCustomMode, uint8 NewCustomMove); //yes, that's a typo. Just staying true to the ALS.

	void OnALSMovementModeChanged();

	//RotationSystem

	UFUNCTION(BlueprintCallable, Category = "GASCMC")
	void SetCharacterRotation(FRotator InTargetRotation, bool InbInterpRotation, float InInterpSpeed);

	UFUNCTION(BlueprintCallable, Category = "GASCMC")
	void LockCharacterRotationToLookingPoint();

	void AddCharacterRotation(FRotator AddAmount);

	FRotator LookingDirectionWithOffset(float OffsetInterpSpeed, float NEAngle, float NWAngle, float SEAngle, float SWAngle, float Buffer);

	float CalculateRotationRate(float SlowSpeed, float SlowSpeedRate, float FastSpeed, float FastSpeedRate);

	//MovementSystem

	void UpdateALSCharacterMovementSettings();

	void CustomAcceleration();

	//Input

	void PlayerMovementInput(bool InbIsForwardAxis);

	//ALS Interface
	void ALS_AddCharacterRotation(FRotator AddAmount);

protected:

    virtual void BeginPlay() override;


    UPROPERTY(BlueprintReadWrite, Category = "GASCharacterMovementComponent")
    AAgoraCharacterBase* OwningGASCharacter = nullptr;

	//////////////////////////////////////////////////////////////////////////
	// ALS Base Character Properties (note that some are public and some are not)
	//////////////////////////////////////////////////////////////////////////

	//Input

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float LookUpDownRate = 150.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float LookLeftRightRate = 150.0f;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float ForwardAxisValue = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float RightAxisValue = 0.0f;

	//Essential
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Agora|ALSCharacter")
	FVector MovementInput;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float AimYawDelta = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float AimYawRate = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float MovementInputVelocityDifference = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float TargetCharacterRotationDifference = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float Direction = 0.0f;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Agora|ALSCharacter")
	FRotator CharacterRotation;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Agora|ALSCharacter")
	FRotator LookingRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	FRotator LastVelocityRotation;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Agora|ALSCharacter")
	FRotator TargetRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	FRotator LastMovementInputRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	bool bTurnInPlaceEnabled = true;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	EALS_MovementMode ALS_MovementMode = EALS_MovementMode::Grounded;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	EALS_MovementMode ALS_PrevMovementMode = EALS_MovementMode::None;

protected:
	//RotationSystem
	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	ECardinalDirection CardinalDirection = ECardinalDirection::North;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	FRotator JumpRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float RotationOffset = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float RotationRateMultiplier = 1.0f;
public:

	// Movement System
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float WalkingSpeed = 145.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float RunningSpeed = 700.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float SprintingSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float WalkingAcceleration = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float RunningAcceleration = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float WalkingDeceleration = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float RunningDeceleration = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float WalkingGroundFriction = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Agora|ALSCharacter")
	float RunningGroundFriction = 6.0f;

protected:

	//Bone Names
	UPROPERTY(BlueprintReadWrite, Category = "Agora|ALSCharacter")
	FName PelvisBone = FName("pelvis");


	//ALS Character "Macros"
	bool CardinalDirectionAngles(float Value, float Min, float Max, float Buffer, ECardinalDirection InCardinalDirection);

	UAnimInstance* AnimInstance();

	UAnimInstance* PostProcessInstance();

	TScriptInterface<IAgoraALSInterface> AnimInstanceAsALSInterface();

	TScriptInterface<IAgoraALSInterface> PostProcessInstanceAsALSInterface();

	bool IsNetworked();

	bool IsLocallyControlled();

	//ALS Character Collapsed Graph Functions
	//this is just to keep things organized and maintain as close of a 1-1 correspondence between the BP and this code.
	void DoEveryFrame();

	void DoWhileGrounded();

	void ManageCharacterRotation();

	void Grounded();

	void Falling();
};



