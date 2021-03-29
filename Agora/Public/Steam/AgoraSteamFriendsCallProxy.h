// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once


#include "CoreMinimal.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "AgoraSteamLibrary.h"
#include "OnlineSubsystemUtils.h"
#include "OnlinePresenceInterface.h"
#include "AgoraSteamFriendsCallProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamFriendsRetrieved, const TArray<FAgoraSteamFriend>&, SteamFriends);

class APlayerController;

/**
 * Proxy to retrieve steam friends list.
 */
UCLASS()
class AGORA_API UAgoraSteamFriendsCallProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_BODY()

	TWeakObjectPtr<APlayerController> PlayerController;
	UObject* WorldContextObject;
	FOnReadFriendsListComplete OnReadFriendsListComplete;

public:
	UAgoraSteamFriendsCallProxy(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FOnSteamFriendsRetrieved OnSteamFriendsRetrievedSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnSteamFriendsRetrieved OnSteamFriendsRetrievedFailure;


	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "AgoraSteam")
	static UAgoraSteamFriendsCallProxy* GetSteamFriends(UObject* WorldContextObject, APlayerController* PlayerController);

	void OnGetSteamFriendsListComplete(int32 LocalUserId, bool bSuccessful, const FString& ListName, const FString& ErrorMessage);

	virtual void Activate() override;
};
