// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OnlinePresenceInterface.h"
#include "AgoraSteamLibrary.generated.h"

USTRUCT(BlueprintType)
struct FAgoraSteamFriend
{
	GENERATED_BODY()

	FAgoraSteamFriend() {}

	UPROPERTY(BlueprintReadOnly, Category = "AgoraSteam")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraSteam")
	FString RealName;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraSteam")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraSteam")
	bool bIsOnline;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraSteam")
	bool bIsPlaying;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraSteam")
	bool bIsPlayingSameGame;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraSteam")
	bool bIsJoinable;

	UPROPERTY(BlueprintReadOnly, Category = "AgoraSteam")
	bool bHasVoiceSupport;

	EOnlinePresenceState::Type PresenceState;

	TSharedPtr<const FUniqueNetId> UniqueNetId;
};

/**
 * Functions and types related to the online steam subsystem.
 */
UCLASS()
class AGORA_API UAgoraSteamLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
