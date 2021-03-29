// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraSteamFriendsCallProxy.h"
#include "Agora.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

UAgoraSteamFriendsCallProxy::UAgoraSteamFriendsCallProxy(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer), 
	OnReadFriendsListComplete(FOnReadFriendsListComplete::CreateUObject(this, &UAgoraSteamFriendsCallProxy::OnGetSteamFriendsListComplete))
{

}

UAgoraSteamFriendsCallProxy* UAgoraSteamFriendsCallProxy::GetSteamFriends(UObject* WorldContextObject, APlayerController* PlayerController)
{
	UAgoraSteamFriendsCallProxy* Proxy = NewObject<UAgoraSteamFriendsCallProxy>();
	Proxy->WorldContextObject = WorldContextObject;
	Proxy->PlayerController = PlayerController;
	return Proxy;
}

void UAgoraSteamFriendsCallProxy::OnGetSteamFriendsListComplete(int32 LocalUserId, bool bSuccessful, const FString& ListName, const FString& ErrorMessage)
{
	TArray<FAgoraSteamFriend> SteamFriends;

	if (bSuccessful)
	{
		IOnlineFriendsPtr FriendInterface = Online::GetFriendsInterface();
		if (FriendInterface.IsValid())
		{
			TArray<TSharedRef<FOnlineFriend>> FriendsList;
			FriendInterface->GetFriendsList(LocalUserId, ListName, FriendsList);

			for (TSharedRef<FOnlineFriend> Friend : FriendsList)
			{
				FOnlineUserPresence Presence = Friend->GetPresence();
				FAgoraSteamFriend SteamFriend;
				SteamFriend.DisplayName = Friend->GetDisplayName();
				SteamFriend.RealName = Friend->GetRealName();
				SteamFriend.Status = Presence.Status.StatusStr;
				SteamFriend.bIsOnline = Presence.bIsOnline;
				SteamFriend.bIsPlaying = Presence.bIsPlaying;
				SteamFriend.bIsPlayingSameGame = Presence.bIsPlayingThisGame;
				SteamFriend.PresenceState = (EOnlinePresenceState::Type) (int32) (Presence.Status.State);
				SteamFriend.UniqueNetId = Friend->GetUserId();

				SteamFriends.Emplace(SteamFriend);
			}

			OnSteamFriendsRetrievedSuccess.Broadcast(SteamFriends);
		}
	}

	OnSteamFriendsRetrievedFailure.Broadcast(SteamFriends);
}

void UAgoraSteamFriendsCallProxy::Activate()
{
	if (PlayerController.IsValid())
	{
		IOnlineFriendsPtr FriendInterface = Online::GetFriendsInterface();
		if (FriendInterface.IsValid())
		{
			const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
			FriendInterface->ReadFriendsList(LocalPlayer->GetControllerId(), EFriendsLists::ToString(EFriendsLists::Default), OnReadFriendsListComplete);
			return;
		}
	}

	TArray<FAgoraSteamFriend> SteamFriends;
	OnSteamFriendsRetrievedFailure.Broadcast(SteamFriends);
}
