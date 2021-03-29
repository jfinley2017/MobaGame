// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraGameLiftService.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Agora.h"
#include "GameLiftClientApi.h"
#include "GameLiftClientObject.h"
#include "Kismet/GameplayStatics.h"
#include "AgoraBlueprintFunctionLibrary.h"


UAgoraGameLiftService::UAgoraGameLiftService(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}


void UAgoraGameLiftService::StartMatchmaking()
{
	TRACE(Agora, Log, "Started matchmaking");
	ResetState();
	PlayerId = FGuid::NewGuid().ToString();
	TArray<FGameLiftPlayer> Players;
	FGameLiftPlayer Player;
	Player.Elo = 1200;
	Player.LatencyMilliseconds = 50;
	Player.PlayerId = PlayerId;
	Players.Add(Player);
	FGameLiftMatchmakingRequest Request;
	Request.TicketId = FGuid::NewGuid().ToString();
	Request.Players = Players;
	Request.ConfigurationName = MatchmakingConfigName;
	UGameLiftStartMatchmaking* StartMatchmakingObject = UGameLiftClientObject::StartMatchmaking(Request);
	StartMatchmakingObject->OnStartMatchmakingSuccess.AddDynamic(this, &UAgoraGameLiftService::OnStartMatchmakingSuccess);
	StartMatchmakingObject->OnStartMatchmakingFailed.AddDynamic(this, &UAgoraGameLiftService::OnStartMatchmakingFailure);
	StartMatchmakingObject->Activate();
	GetWorld()->GetTimerManager().SetTimer(MatchmakingPollingTimer, this, &UAgoraGameLiftService::DescribeMatchmaking, 5.0f, true);
}

void UAgoraGameLiftService::StopMatchmaking()
{
	UGameLiftStopMatchmaking* StopMatchmakingObject = UGameLiftClientObject::StopMatchmaking(MatchmakingTicketId);
	StopMatchmakingObject->OnStopMatchmakingSuccess.AddDynamic(this, &UAgoraGameLiftService::OnStopMatchmakingSuccess);
	StopMatchmakingObject->OnStopMatchmakingFailed.AddDynamic(this, &UAgoraGameLiftService::OnStopMatchmakingFailure);
	StopMatchmakingObject->Activate();
}

void UAgoraGameLiftService::AcceptMatch()
{
	TArray<FString> PlayerIds;
	PlayerIds.Add(PlayerId);
	UGameLiftAcceptMatch* AcceptMatchObject = UGameLiftClientObject::AcceptMatch(PlayerIds, MatchmakingTicketId);
	AcceptMatchObject->OnAcceptMatchSuccess.AddDynamic(this, &UAgoraGameLiftService::OnAcceptMatchSuccess);
	AcceptMatchObject->OnAcceptMatchFailed.AddDynamic(this, &UAgoraGameLiftService::onAcceptMatchFailure);
	AcceptMatchObject->Activate();
}

void UAgoraGameLiftService::RejectMatch()
{
	TArray<FString> PlayerIds;
	PlayerIds.Add(PlayerId);
	UGameLiftAcceptMatch* RejectMatchObject = UGameLiftClientObject::RejectMatch(PlayerIds, MatchmakingTicketId);
	RejectMatchObject->OnAcceptMatchSuccess.AddDynamic(this, &UAgoraGameLiftService::OnRejectMatchSuccess);
	RejectMatchObject->OnAcceptMatchFailed.AddDynamic(this, &UAgoraGameLiftService::OnRejectMatchFailure);
	RejectMatchObject->Activate();
}

void UAgoraGameLiftService::DescribeMatchmaking()
{
	TArray<FString> TicketIds;
	TicketIds.Add(MatchmakingTicketId);
	UGameLiftDescribeMatchmaking* DescribeMatchmakingObject = UGameLiftClientObject::DescribeMatchmaking(TicketIds);
	DescribeMatchmakingObject->OnDescribeMatchmakingSuccess.AddDynamic(this, &UAgoraGameLiftService::OnDescribeMatchmakingSuccess);
	DescribeMatchmakingObject->OnDescribeMatchmakingFailed.AddDynamic(this, &UAgoraGameLiftService::OnDescribeMatchmakingFailure);
	DescribeMatchmakingObject->Activate();
}

void UAgoraGameLiftService::OnStartMatchmakingSuccess(const FGameLiftMatchmakingResponseTicket& Ticket)
{
	MatchmakingTicketId = Ticket.TicketId;
}

void UAgoraGameLiftService::OnStartMatchmakingFailure(const FString& ErrorMessage)
{
	TRACE(Agora, Log, "Error OnStartMatchmaking failure: %s", *ErrorMessage);
}

void UAgoraGameLiftService::OnStopMatchmakingSuccess()
{
	TRACE(Agora, Log, "Stopped matchmaking request");
	ResetState();
	OnMatchmakingStopped.Broadcast();
}

void UAgoraGameLiftService::OnStopMatchmakingFailure(const FString& ErrorMessage)
{
	TRACE(Agora, Log, "Error OnStopMatchmaking failure: %s", *ErrorMessage);
}

void UAgoraGameLiftService::OnDescribeMatchmakingSuccess(const TArray<FGameLiftMatchmakingResponseTicket>& Tickets)
{
	for (const FGameLiftMatchmakingResponseTicket& Ticket : Tickets)
	{
		if (Ticket.TicketId == MatchmakingTicketId)
		{
			TRACE(Agora, Log, "DescribeMM: %d", Ticket.Status);
			switch (Ticket.Status)
			{
			case EGameLiftMatchmakingTicketStatus::REQUIRES_ACCEPTANCE:
			{
				if (!bPromptedForAccept)
				{
					//set a UI timer here: they can't wait around to accept forever.
					MatchFoundTimeoutTimer = FAgoraCountDownTimer(GetWorld(), 30.f);
					MatchFoundTimeoutTimer.SetOnTickLambda([&] {
						OnMatchmakingAcceptTimeoutTick.Broadcast( (int32) MatchFoundTimeoutTimer.GetTimeRemaining());
					});
					MatchFoundTimeoutTimer.SetOnFinishLambda([&] {
						OnMatchmakingAcceptTimedOut.Broadcast();
					});
					MatchFoundTimeoutTimer.Start();
					bPromptedForAccept = true;
					OnMatchFound.Broadcast();
				}
				break;
			}
			case EGameLiftMatchmakingTicketStatus::COMPLETED:
			{
				GetWorld()->GetTimerManager().ClearTimer(MatchmakingPollingTimer);
				for (const FGameLiftMatchedPlayerSession& PlayerSession : Ticket.GameSessionConnectionInfo.MatchedPlayerSessions)
				{
					FString LevelName = Ticket.GameSessionConnectionInfo.IpAddress + ":";
					LevelName.AppendInt(Ticket.GameSessionConnectionInfo.Port);
					TMap<FString, FString> OptionsMap;
					OptionsMap.Add("PlayerId", PlayerSession.PlayerId);
					OptionsMap.Add("PlayerSessionId", PlayerSession.PlayerSessionId);
					FString OptionsString = UAgoraBlueprintFunctionLibrary::CreateOptionsStringForOpenLevel(OptionsMap);

					FString GameInfoToSave = LevelName + "," + PlayerSession.PlayerSessionId;
					UAgoraBlueprintFunctionLibrary::SaveTextFile(GameLiftFileName, GameInfoToSave);

					UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName), true, OptionsString);
				}
				break;
			}
			case EGameLiftMatchmakingTicketStatus::TIMED_OUT:
			{
				OnMatchmakingAcceptTimedOut.Broadcast();
				ResetState();
				break;
			}
			}
		}
	}
}

void UAgoraGameLiftService::OnDescribeMatchmakingFailure(const FString& ErrorMessage)
{
	TRACE(Agora, Log, "Error OnDescribeMatchmaking failure: %s", *ErrorMessage);
}

void UAgoraGameLiftService::OnAcceptMatchSuccess()
{
	TRACE(Agora, Log, "Accepted match");
}

void UAgoraGameLiftService::onAcceptMatchFailure(const FString& ErrorMessage)
{
	TRACE(Agora, Log, "Error OnAcceptMatch failure: %s", *ErrorMessage);
}

void UAgoraGameLiftService::OnRejectMatchSuccess()
{
	TRACE(Agora, Log, "Rejected match");
	ResetState();
}

void UAgoraGameLiftService::OnRejectMatchFailure(const FString& ErrorMessage)
{
	TRACE(Agora, Log, "Error OnRejectMatch failure: %s", *ErrorMessage);
}

void UAgoraGameLiftService::ResetState()
{
	PlayerId = "";
	MatchmakingTicketId = "";
	bPromptedForAccept = false;
	GetWorld()->GetTimerManager().ClearTimer(MatchmakingPollingTimer);
	MatchFoundTimeoutTimer.Cancel();
}