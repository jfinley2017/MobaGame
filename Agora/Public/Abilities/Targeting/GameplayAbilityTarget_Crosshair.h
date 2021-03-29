// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameplayAbilityTarget_Crosshair.generated.h"

class AAgoraCharacterBase;


/************************************************************************/
/* A targeting actor used to send the crosshair position of the player  */
/* to the server.                                                       */
/************************************************************************/

USTRUCT(BlueprintType)
struct FGameplayAbilityTargeting_Crosshair : public FGameplayAbilityTargetData
{
    GENERATED_BODY()
    
    // The socket(usually) location of where we want our origin to be
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
    FGameplayAbilityTargetingLocationInfo FiringLocation;

    // The hit result of our crosshair (formally known as the player's (usually) aim intention)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
    FGameplayAbilityTargetingLocationInfo CrosshairHitLocation;

    // Including this here because there might be some cases in the future where we may want
    // to keep track of how much time a player charged a crosshair-targeted ability
    // As of right now its not even tracked (because we currently do not have a way to track charging on the character)
    // @todo: ^
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting")
    float ChargeTime;

    // THIS IS NEEDED. IT WONT BE VALID ON SERVER IF THIS IS NOT HERE!!!
    UPROPERTY()
    uint32 UniqueID;

    virtual bool HasOrigin() const override
    {
        return true;
    }

    virtual FTransform GetOrigin() const override 
    {
        return FiringLocation.GetTargetingTransform();
    }

    virtual bool HasEndPoint() const override
    {
        return true;
    }

    virtual FVector GetEndPoint() const override
    {
        return CrosshairHitLocation.GetTargetingTransform().GetLocation();
    }

    virtual FTransform GetEndPointTransform() const override
    {
        return CrosshairHitLocation.GetTargetingTransform();
    }

    virtual UScriptStruct* GetScriptStruct() const override
    {
        return FGameplayAbilityTargeting_Crosshair::StaticStruct();
    }

    virtual FString ToString() const override
    {
        return TEXT("FGameplayAbilityTargeting_Crosshair");
    }

    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargeting_Crosshair> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargeting_Crosshair>
{
    enum
    {
        WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
    };
};

UCLASS()
class AGORA_API AGameplayAbilityTarget_Crosshair : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:
    virtual void ConfirmTargetingAndContinue() override;
	
protected:

    AAgoraCharacterBase* OwningCharacter = nullptr;

    // Used as an origin point for the target data
    UPROPERTY(BlueprintReadWrite, Category = "TargetCrosshair", meta = (ExposeOnSpawn="true"))
    FTransform FiringLocation;


};
