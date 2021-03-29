// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#include "AgoraTowerBase.h"
#include "AbilitySystemBlueprintLibrary.h" // Get ASC
#include "GameplayEffectTypes.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayEffect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h" 
#include "Net/UnrealNetwork.h"

#include "Agora.h"
#include "AgoraTypes.h"
#include "TeamInterface.h"
#include "AgoraMinionBase.h"
#include "AgoraGameMode.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraAttributeSetBase.h"
#include "Components/AgoraTowersStateComponent.h"
#include "Components/AgoraTimeLevelComp.h"
#include "AgoraAbilitySystemComponent.h"
#include "AgoraGameplayTags.h"
#include "Components/AgoraVisionComponent.h"
#include "AgoraAbilityLibrary.h"
#include "Lib/AgoraTeamLibrary.h"


AAgoraTowerBase::AAgoraTowerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Use a sphere as a simple collision representation
	TurretArea = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	TurretArea->SetupAttachment(RootComponent);

	TurretArea->SetCollisionProfileName("OverlapOnlyPawn");

    VisionComponent->SetSphereRadius(2000.0f);
}

void AAgoraTowerBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAgoraTowerBase, EnemyTarget);
    DOREPLIFETIME(AAgoraTowerBase, bHasTargeted);
}

void AAgoraTowerBase::BeginPlay()
{
    Super::BeginPlay();

    TurretArea->OnComponentBeginOverlap.AddDynamic(this, &AAgoraTowerBase::NotifyTurretAreaOverlapBegin);
    TurretArea->OnComponentEndOverlap.AddDynamic(this, &AAgoraTowerBase::NotifyTurretAreaOverlapEnd);

    //Check for the team meshes to apply based on team
    if(ensure(DuskTeamMesh) && ensure(DawnTeamMesh))
    {
        if (GetTeam() == ETeam::Dawn)
        {
            GetMesh()->SetSkeletalMesh(DawnTeamMesh);
        }
        else if (GetTeam() == ETeam::Dusk)
        {
            GetMesh()->SetSkeletalMesh(DuskTeamMesh);
        }
    }
}

void AAgoraTowerBase::StopPostDeath()
{
    Super::StopPostDeath();
    bShouldTarget = false;
    SetEnemyTarget(nullptr);
}

bool AAgoraTowerBase::ValidateEnemies()
{
	bool bFoundInvalid = false;

	for (AActor* Enemy : EnemyMinions)
	{
		if (!ValidateEnemy(Enemy))
		{
			EnemyMinions.Remove(Enemy);
			bFoundInvalid = true;
		}
	}

	for (AActor* Enemy : EnemyHeroes)
	{
		if (!ValidateEnemy(Enemy))
		{
			EnemyHeroes.Remove(Enemy);
			bFoundInvalid = true;
		}
	}

	return bFoundInvalid;
}

bool AAgoraTowerBase::ValidateEnemy(AActor* Enemy)
{
	if (!Enemy)
	{
		return false;
	}

	IGameplayTagAssetInterface* AssetTagEnemy = Cast<IGameplayTagAssetInterface>(Enemy);
	if (!AssetTagEnemy)
	{
		return false;
	}

	bool bIsDead = AssetTagEnemy->HasMatchingGameplayTag(UAgoraGameplayTags::Dead());
	bool bIsFriendly = UAgoraTeamLibrary::IsFriendly(this, Enemy);

	/*float TargetAreaRadius = TurretArea->GetScaledSphereRadius();
	float DistanceSquared = FVector::DistSquared(Enemy->GetActorLocation(), GetActorLocation());

	bool bOutOfRange = DistanceSquared < (TargetAreaRadius * TargetAreaRadius);*/

	return !(bIsFriendly || bIsDead);
}

void AAgoraTowerBase::SetEnemyTarget(AActor* Target)
{	
	EnemyTarget = Cast<APawn>(Target);

	GetAbilitySystemComponent()->RemoveGameplayCue(TargetingCue);

	if (Target)
	{
		FVector BeamOrigin = GetMesh()->GetSocketLocation("TargetingBeam");
		
		UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

		FGameplayCueParameters CueParams;
		CueParams = UAgoraBlueprintFunctionLibrary::MakeTargetedGameplayCueParams(ASC, Target, this, BeamOrigin);

		ASC->AddGameplayCue(TargetingCue, CueParams);
	}
}

void AAgoraTowerBase::FinishPlayDying()
{
	Super::FinishPlayDying();

    SetActorHiddenInGame(true);
}

/** Rules for targeting priorities */
AActor* AAgoraTowerBase::FindTarget()
{
    if (IsDead()) { return nullptr; }

	//if last enemy hero is already set than skip find target and fire again
	if (ValidateEnemy(EnemyTarget))
	{
		return EnemyTarget;
	}
	else
	{
		// Check all our targets and remove invalid ones before picking one
		ValidateEnemies();
	}

	// if there are no minions and any heroes
	if(EnemyMinions.Num() == 0 && EnemyHeroes.Num() > 0)
	{
        SetEnemyTarget(EnemyHeroes[0]);
        UE_LOG(AgoraAI, Display, TEXT("Turret targeting enemy hero %s"), *EnemyTarget->GetName());
	}
	else if (EnemyMinions.Num() > 0)
	{
		SetEnemyTarget(EnemyMinions[0]);
		UE_LOG(AgoraAI, Display, TEXT("Turret targeting enemy minion %s"), *EnemyTarget->GetName());
	}
	else
	{
		SetEnemyTarget(nullptr);
	}


	return EnemyTarget;
}

void AAgoraTowerBase::SetInvulnerable(bool bMakeInvuln /*= true*/)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (!bMakeInvuln)
	{
		TRACE(AgoraAI, Log, "Tower set vulnerable");
		ASC->RemoveActiveEffectsWithTags(InvulnerableTag);
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InvulnerableEffect, 0, ASC->MakeEffectContext());
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

}

/** Called when a friendly hero is hit. */
void AAgoraTowerBase::SetAggressiveEnemy(AActor * Enemy)
{
	if (EnemyHeroes.Contains(Enemy))
	{
		//make sure that the turret wasn't already targeting an enemy hero
		if (!EnemyTarget || UAgoraGameplayTags::IsMinion(EnemyTarget))
		{
			UE_LOG(AgoraAI, Display, TEXT("Turret targeting hero over minion due to player hit"));
			SetEnemyTarget(Enemy);
		}
	}
}

//----------------------Array Management---------------------------//
void AAgoraTowerBase::AddHero(AActor* Hero)
{
	if (!UAgoraBlueprintFunctionLibrary::IsFriendly(Hero, this))
	{
		UE_LOG(AgoraAI, Display, TEXT("Turret tracking enemy hero %s"), *Hero->GetName());

		EnemyHeroes.AddUnique(Hero);
		FindTarget();
	}
	else
	{
		UE_LOG(AgoraAI, Display, TEXT("Turret did not add hero %s"), *Hero->GetName());
	}
}
void AAgoraTowerBase::AddEnemyMinion(AActor* Minion)
{
	if (!UAgoraBlueprintFunctionLibrary::IsFriendly(Minion, this))
	{
		EnemyMinions.AddUnique(Minion);
		FindTarget();
	}
}

void AAgoraTowerBase::AddTrackedActor(AActor* const Target)
{
	if (UAgoraGameplayTags::IsMinion(Target))
	{
		AddEnemyMinion(Target);
	}

	if (UAgoraGameplayTags::IsHero(Target))
	{
		AddHero(Target);
	}
}

void AAgoraTowerBase::RemoveTrackedActor(AActor* const Target)
{
	if (UAgoraGameplayTags::IsMinion(Target))
	{
		RemoveEnemyMinion(Target);
	}

	if (UAgoraGameplayTags::IsHero(Target))
	{
		RemoveHero(Target);
	}
}

void AAgoraTowerBase::RemoveHero(AActor* Hero)
{
	if (!UAgoraBlueprintFunctionLibrary::IsFriendly(Hero, this))
	{
		if (EnemyHeroes.Contains(Hero))
		{
			EnemyHeroes.Remove(Hero);
		}

		ResetIfTargetRemoved(Hero);
	}
}

void AAgoraTowerBase::RemoveEnemyMinion(AActor* Minion)
{
	if (!UAgoraBlueprintFunctionLibrary::IsFriendly(Minion, this))
	{
		if (EnemyMinions.Contains(Minion))
		{
			EnemyMinions.Remove(Minion);
		}

		ResetIfTargetRemoved(Minion);
	}
}

// Reset EnemyTarget if the given target is the EnemyTarget
void AAgoraTowerBase::ResetIfTargetRemoved(AActor* const Target)
{
	if (EnemyTarget == Target)
	{
		SetEnemyTarget(nullptr);
		FindTarget();
	}
}

bool AAgoraTowerBase::CheckForEnemyMinion()
{
	return (EnemyMinions.Num() > 0);
}

TArray<AActor*> AAgoraTowerBase::GetAllEnemies()
{
	TArray<AActor*> AllEnemies;
	AllEnemies.Append(EnemyHeroes);
	AllEnemies.Append(EnemyMinions);
	return AllEnemies;
}

//----------------------Tower Area-----------------------//
void AAgoraTowerBase::NotifyTurretAreaOverlapBegin(class UPrimitiveComponent* OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (HasAuthority() && !IsDead() && bShouldTarget)
    {
        UE_LOG(AgoraAIVerbose, Display, TEXT("Turret overlapped actor %s"), *OtherActor->GetName());


        //Don't remove the hero if he left and came back in before turret finishes fire
        if (OtherActor == EnemyTarget && GetWorldTimerManager().IsTimerActive(FinishFireHandle))
        {
            GetWorld()->GetTimerManager().ClearTimer(FinishFireHandle);
            return;
        }

		// For sanity, make sure we don't already have this enemy in our tracking. Should only be possible in weird circumstances
		for (AActor* Enemy : GetAllEnemies())
		{
			if (Enemy == OtherActor) { return; }
		}

        AddTrackedActor(OtherActor);
    }
}

// Targeted heroes still get shot if they run out of the turret area
void AAgoraTowerBase::NotifyTurretAreaOverlapEnd(class UPrimitiveComponent* OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	// This can happen when the turret dies
	if (!OtherActor)
	{
		return;
	}

    if (HasAuthority())
    {
        if (EnemyTarget == OtherActor && bHasTargeted)
        {
            bHasTargeted = false;
            FTimerDelegate TimerDel;

            TimerDel.BindLambda([&](AActor* TrackedActor)
            {
                RemoveTrackedActor(TrackedActor);
            }, OtherActor);

            GetWorldTimerManager().SetTimer(FinishFireHandle, TimerDel, (1.5f / FireRate), false);
            return;
        }
        else
        {
            RemoveTrackedActor(OtherActor);
        }
    }
	
}

FString AAgoraTowerBase::ToString()
{
    return FString::Printf(TEXT("Tower team %d lane %d type %d"), (uint8)GetTeam(), (uint8)Lane, (uint8)Type);
}

ETowerType AAgoraTowerBase::GetTowerType() const
{
    return Type;
}

ELaneEnum AAgoraTowerBase::GetLane() const
{
    return Lane;
}

