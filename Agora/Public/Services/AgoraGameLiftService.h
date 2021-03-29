// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameLiftClientTypes.h"
#include "TimerManager.h"
#include "AgoraCountdownTimer.h"
#include "AgoraGameLiftService.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchFound);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchmakingStopped);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchmakingAcceptTimedOut);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingAcceptTimeoutTick, int32, SecondsRemaining);


static FString GameLiftFileName = "game_info.dat";
/**
 * Handles GameLift-related logic. Using an object allows us to hold state, which is necessary for the matchmaking flow control.
 * Doesn't inherit from AInfo (like the other services) because it doesn't need to be replicated at all. In fact it only needs to exist on clients.
 * Lives in AgoraMenuController.
 * Exposes delegates for matchmaking events that the UI should react to.
 * NOTE: This will HAVE to be refactored for parties soon.
 */
UCLASS()
class AGORA_API UAgoraGameLiftService : public UObject
{
	GENERATED_BODY()
	
public:
	UAgoraGameLiftService(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "AgoraGameLiftService")
	void StartMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "AgoraGameLiftService")
	void StopMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "AgoraGameLiftService")
	void AcceptMatch();

	UFUNCTION(BlueprintCallable, Category = "AgoraGameLiftService")
	void RejectMatch();

	UPROPERTY(BlueprintAssignable, Category = "AgoraGameLiftService")
	FOnMatchFound OnMatchFound;

	UPROPERTY(BlueprintAssignable, Category = "AgoraGameLiftService")
	FOnMatchmakingStopped OnMatchmakingStopped;

	UPROPERTY(BlueprintAssignable, Category = "AgoraGameLiftService")
	FOnMatchmakingAcceptTimeoutTick OnMatchmakingAcceptTimeoutTick;

	UPROPERTY(BlueprintAssignable, Category = "AgoraGameLiftService")
	FOnMatchmakingAcceptTimedOut OnMatchmakingAcceptTimedOut;	

protected:
	//constants
	const FString MatchmakingConfigName = "WhetstonePvpMatchmakingConfiguration";
	const FString TeamKey = "dawn";

	//state
	FString PlayerId;
	FString MatchmakingTicketId;
	FTimerHandle MatchmakingPollingTimer;
	bool bPromptedForAccept = false;

	FAgoraCountDownTimer MatchFoundTimeoutTimer;

	UFUNCTION()
	void DescribeMatchmaking();

	UFUNCTION()
	void OnStartMatchmakingSuccess(const FGameLiftMatchmakingResponseTicket& Ticket);
	UFUNCTION()
	void OnStartMatchmakingFailure(const FString& ErrorMessage);

	UFUNCTION()
	void OnStopMatchmakingSuccess();
	UFUNCTION()
	void OnStopMatchmakingFailure(const FString& ErrorMessage);

	UFUNCTION()
	void OnDescribeMatchmakingSuccess(const TArray<FGameLiftMatchmakingResponseTicket>& Tickets);
	UFUNCTION()
	void OnDescribeMatchmakingFailure(const FString& ErrorMessage);

	UFUNCTION()
	void OnAcceptMatchSuccess();
	UFUNCTION()
	void onAcceptMatchFailure(const FString& ErrorMessage);

	UFUNCTION()
	void OnRejectMatchSuccess();
	UFUNCTION()
	void OnRejectMatchFailure(const FString& ErrorMessage);

	void ResetState();
};
