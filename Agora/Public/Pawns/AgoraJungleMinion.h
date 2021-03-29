// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Pawns/AgoraMinionBase.h"
#include "AgoraJungleMinion.generated.h"

class AAgoraJungleCamp;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetUpdated, AActor*, CampTarget);

/**
 * Base class for jungle minions
 */
UCLASS()
class AGORA_API AAgoraJungleMinion : public AAgoraMinionBase
{
	GENERATED_BODY()

public:
	AAgoraJungleMinion(const FObjectInitializer& ObjectInitializer);
	
protected:
	/**
	 * Cache our original spawn point in case we need to reset
	 */
	UPROPERTY(BlueprintReadOnly, Category=AgoraJungleMinion)
	FTransform OriginalSpawnTransform;

	virtual void BeginPlay() override;

	void FullHeal();

	/**
	 * Most of the behavior for resetting is done via BT
	 */
	UFUNCTION(BlueprintCallable, Category = AgoraJungleMinion)
	virtual void FinishReset();

	UFUNCTION(BlueprintCallable, Category = AgoraJungleMinion)
	virtual void StartReset();

	UPROPERTY(BlueprintReadWrite, Category = AgoraJungleMinion)
	bool bIsResetting;

	UFUNCTION(BlueprintCallable)
	void SetTarget(AActor* InTarget);

	UPROPERTY(BlueprintAssignable)
	FOnTargetUpdated OnTargetUpdated;

	// TeamInterface interface
	ETeam GetTeam() const override;
	// ~TeamInterface interface

	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn = true))
	AAgoraJungleCamp* Camp;

	friend class AAgoraJungleCamp;
};
