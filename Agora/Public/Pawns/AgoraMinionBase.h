// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#pragma once

#include "CoreMinimal.h"
#include "AgoraCharacterBase.h"
#include "AgoraMinionBase.generated.h"


UCLASS()
class AGORA_API AAgoraMinionBase : public AAgoraCharacterBase
{
	GENERATED_BODY()

public:

	AAgoraMinionBase(const FObjectInitializer& ObjectInitializer);

    // AAgoraCharacter interface
    virtual void FinishPlayDying() override;
    virtual void FinishDying() override;
    virtual bool IsDead() const override;
    virtual void NotifyDied(const FDamageContext& DamageContext) override;
    // ~AAgoraCharacter interface

    UFUNCTION(BlueprintCallable, Category = "AgoraMinionLane")
    AActor* GetTarget() { return Target; }

    // C++ hook for attacks
    UFUNCTION(BlueprintCallable, Category = "AgoraMinion")
    virtual void Attack();

    UFUNCTION(BlueprintCallable, Category = "AgoraMinion")
    float GetAttackRange() { return AttackRange; }



protected:

    // The current target for this unit. This can be determined in a variety of ways eg priority lists/gameplay-related events
    UPROPERTY(BlueprintReadWrite, Category = "AgoraMinion")
    AActor* Target = nullptr;

    // Blueprint hook for attacks
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "AgoraMinion")
	void OnAttack();

    bool bIsDead = false;

    // This minion's attack range. This should really be set by whatever pooling mechanism we have 
    // eg if the spawners request a range minion, the pooler should set this value accordingly
    UPROPERTY(EditDefaultsOnly, Category = "Agora|Minion")
    float AttackRange;
};