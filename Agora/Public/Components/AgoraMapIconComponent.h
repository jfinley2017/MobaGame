// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/DamageReceiverInterface.h"
#include "AgoraMapIconComponent.generated.h"

UENUM(BlueprintType)
enum class EMapIconDisplayRules : uint8
{
    VisibleWhenInVision,
    AlwaysVisibleRemoveOnDeath
    
};

UENUM(BlueprintType)
enum class EMapIconEnum : uint8
{
    Lowest,
    Towers,
    Minions,
    Heroes,
    Max
};

class AAgoraMapTrackerService;
class UAgoraAffiliatedWidget; 

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGORA_API UAgoraMapIconComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	// Sets default values for this component's properties
	UAgoraMapIconComponent();

    // UActorComponent interface
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    // ~UActorComponent interface

    UFUNCTION()
    void NotifyOwnerDied(const FDamageContext& KillingBlow);

    UFUNCTION()
    void NotifyOwnerRevealed();

    UFUNCTION()
    void NotifyOwnerRespawned();

    UFUNCTION()
    void NotifyOwnerHidden();

    UFUNCTION(BlueprintPure, Category = "AgoraMapIconComponent")
    UAgoraAffiliatedWidget* GetIcon();

    UFUNCTION(BlueprintPure, Category = "AgoraMapIconComponent")
    EMapIconEnum GetDrawLevel();

protected:

    // UActorComponent interface
    virtual void BeginPlay() override;
    // ~UActorComponent interface
	
    UPROPERTY(EditDefaultsOnly, Category = "Agora")
    EMapIconDisplayRules DisplayRules;

    UPROPERTY(EditDefaultsOnly, Category = "Agora")
    TSubclassOf<UAgoraAffiliatedWidget> WidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Agora")
    EMapIconEnum DrawLevel;

    UPROPERTY()
    UAgoraAffiliatedWidget* CachedWidget;

    UPROPERTY()
    AAgoraMapTrackerService* CachedMapTrackerService;


};
