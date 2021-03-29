// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraAbilityEffectArea.generated.h"

class UAbilitySystemComponent;

// Hides the instigator pin, which requires pawn rather than actor
UCLASS(HideCategories=(Actor))
class AGORA_API AAgoraAbilityEffectArea : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	UPrimitiveComponent* Collider = nullptr;

	// If true, remove effects at the end of this area application, OR if actors leave the area
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ExposeOnSpawn = true))
	bool bRemoveEffectOnEnd = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	float Duration = 5.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	bool bIsInfinite = false;

	// Filter what this effect will apply to
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	FAgoraSweepQueryParams QueryParams;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	AActor* EffectCauser = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	FGameplayEffectSpecHandle Effect;

	UFUNCTION() // these have to be UFUNCTION's because they're multicast delegate handlers
	virtual void HandleAreaOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	virtual void HandleAreaOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:

    // AActor
    virtual void BeginPlay() override;
    // ~AActor

    void SetupOverlaps();

    void SetupAreaEnd();

    UFUNCTION(BlueprintCallable, Category = "Agora")
    void ApplyEffectToActor(AActor* OtherActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilityEffectArea")
    void OnApplyEffectToActor(AActor* OtherActor);

    UFUNCTION()
    void RemoveEffectsFromActorASC(UAbilitySystemComponent* ASC);


private:
	TMap<UAbilitySystemComponent*, FActiveGameplayEffectHandle> ActiveEffects;
};
