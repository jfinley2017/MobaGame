// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AgoraCheatManager.generated.h"

class AAgoraLaneMinion;
class APawn;
class UUserWidget;

UCLASS(Within = PlayerController)
class AGORA_API UAgoraCheatManager : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UAgoraCheatManager();
	TSubclassOf<AAgoraLaneMinion> MinionClass;

	UFUNCTION(Exec)
	void AgSlomo(float Dilation);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSlomo(float Dilation);

    UFUNCTION(Exec)
    void AgKill();

    UFUNCTION(Exec)
    void AgGrantItem(const FString& ItemName);
    UFUNCTION(Server,Reliable,WithValidation)
    void ServerGrantItem(const FString& ItemName);

    UFUNCTION(Exec)
    void AgRemoveItem(const FString& ItemName);
    UFUNCTION(Server,Reliable,WithValidation)
    void ServerRemoveItem(const FString& ItemName);

    UFUNCTION(Exec)
    void AgGrantItems(const FString& Item1, const FString& Item2, const FString& Item3, const FString& Item4, const FString& Item5, const FString& Item6);
    UFUNCTION(Exec)
    void AgRemoveItems();

	UFUNCTION(Exec)
	void AgSuicide();

	UFUNCTION(Exec)
	void AgSpawnMinion(bool bSameTeam = true);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSpawnMinion(APawn* Pawn, FVector TargetLocation, bool bSameTeam);

	UFUNCTION(Exec)
	void AgIncreaseLevel();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerIncreaseLevel();

	// Make your character OP
	UFUNCTION(Exec)
	void AgGodMode();

	UFUNCTION(Exec)
	void AgSetAttribute(const FString& AttributeName, float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetAttribute(APawn* Pawn, const FString& AttributeName, float Value);

	UFUNCTION(Exec)
	void AgToggleTeam();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerToggleTeam(APawn* Pawn);
	
	UFUNCTION(Exec)
	void AgTeleport();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAgTeleport(APawn* Pawn, FVector TargetLocation);

private:
	bool GetLookLocation(FHitResult& Result);
};
