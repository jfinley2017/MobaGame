// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "AgoraAttributeSetHealth.h"
#include "AgoraBlueprintFunctionLibrary.h" // SendGameplayEvent
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h" 
#include "GameplayEffectExtension.h" // DamageModCallbackData
#include "Agora.h" // Logging
#include "AgoraDataSingleton.h" // death gameplay effect class
#include "AgoraTypes.h" // gameplay tags
#include "Interfaces/DamageReceiverInterface.h"
#include "Interfaces/DamageDealerInterface.h"
#include "AgoraGameMode.h"
#include "AgoraGameplayTags.h"
#include "AgoraStatTrackingLibrary.h"

UAgoraAttributeSetHealth::UAgoraAttributeSetHealth(): 
    Health(1.0f),
    MaxHealth(0.0f),
    HealthRegen(0.0f),
    PhysicalDefense(0.0f),
    MagicalDefense(0.0f)
{

}

void UAgoraAttributeSetHealth::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetHealth, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetHealth, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetHealth, HealthRegen, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetHealth, PhysicalDefense, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetHealth, MagicalDefense, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME(UAgoraAttributeSetHealth, LastTakeHitInfo);
}


void UAgoraAttributeSetHealth::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetMaxHealthAttribute())
    {
        AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
    }
}

void UAgoraAttributeSetHealth::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data)
{
    Super::PostGameplayEffectExecute(Data);

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    FGameplayAttribute ModifiedDataAttribute = Data.EvaluatedData.Attribute;

    if (ModifiedDataAttribute == GetHealthAttribute())
    {
        TRACESTATIC(Agora, Warning, "GameplayEffectSpec: %s modified Health. Use Damage or Healing meta attributes instead", *Data.EffectSpec.ToSimpleString())
    }

    if (ModifiedDataAttribute == GetDamageAttribute() && GetHealth() > 0.0f)
    {
        TakeDamage(Data);
    }

    if (ModifiedDataAttribute == GetHealingAttribute() && GetHealth() > 0.0f)
    {
        // Clamp the Health to MaxHealth

        const float LocalHealingDone = GetHealing();
        SetHealing(0.f);

        if (LocalHealingDone > 0)
        {
            // Apply the health change and then clamp it
            const float OldHealth = GetHealth();
            const float LocalMaxHealth = GetMaxHealth();
            if (OldHealth < LocalMaxHealth)
            {
                SetHealth(FMath::Clamp(OldHealth + LocalHealingDone, 0.0f, GetMaxHealth()));
            }
        }
    }
}

void UAgoraAttributeSetHealth::TakeDamage(const struct FGameplayEffectModCallbackData& Data)
{
    float DamageMagnitude = GetDamage();
    SetDamage(0.0f);

    // Nothing to do.
    if (DamageMagnitude == 0.0f) { return; }

    // Dont want to heal here, print a warning that this shouldnt be happening
    if (DamageMagnitude < 0.0f)
    { 
        TRACE(AgoraCharacter, Warning, "Recieved < 0.0f damage. This should not be the case. Context: %s", *Data.EffectSpec.GetEffectContext().ToString())
        return; 
    }

    // Reduce health
    SetHealth(FMath::Clamp(GetHealth() - DamageMagnitude, 0.0f, GetMaxHealth()));

    // Goal:
    // 'Eat' meta-attribute corresponding to the damage type, applying the value there to health
    // Send event to ASC saying we took damage of type X
    // Notify observers that damage was taken, on both client and server

    FGameplayEffectContextHandle DamagingContextHandle = Data.EffectSpec.GetEffectContext();
    AActor* InstigatorActor = DamagingContextHandle.GetOriginalInstigator();
    AActor* TargetActor = GetOwningActor();
    TScriptInterface<IDamageDealerInterface> DamageDealer = InstigatorActor;
    TScriptInterface<IDamageReceiverInterface> DamageReceiver = GetOwningActor();
   
    // Initialize our GameplayEventData
    FGameplayEventData DamageEventData;
    DamageEventData.ContextHandle = DamagingContextHandle;
    DamageEventData.EventMagnitude = DamageMagnitude;
    DamageEventData.Instigator = InstigatorActor;
    DamageEventData.Target = TargetActor;
    UAgoraBlueprintFunctionLibrary::GetOwnedGameplayTags(InstigatorActor, DamageEventData.InstigatorTags);
    UAgoraBlueprintFunctionLibrary::GetOwnedGameplayTags(TargetActor, DamageEventData.TargetTags);

    // Fire the events for both the instigator and the target (us)
    DamageEventData.EventTag = UAgoraGameplayTags::DamageInstigated();
    UAgoraBlueprintFunctionLibrary::SendGameplayEventToActor(InstigatorActor, UAgoraGameplayTags::DamageInstigated(), DamageEventData);
    APlayerState* InstigatorActorPlayerState = GetPlayerState(InstigatorActor);
    if (InstigatorActorPlayerState)
    {
        UAgoraStatTrackingLibrary::ModifyPlayerStat(InstigatorActorPlayerState, "Damage", EStatModType::Add, DamageMagnitude, true);
    }

    DamageEventData.EventTag = UAgoraGameplayTags::DamageReceived();
    UAgoraBlueprintFunctionLibrary::SendGameplayEventToActor(TargetActor, UAgoraGameplayTags::DamageReceived(), DamageEventData);
    APlayerState* TargetActorPlayerState = GetPlayerState(TargetActor);
    if (TargetActorPlayerState)
    {
        UAgoraStatTrackingLibrary::ModifyPlayerStat(TargetActorPlayerState, "DamageTaken", EStatModType::Add, DamageMagnitude, true);
    }

    FDamageContext DamageContext = FDamageContext(DamageEventData);
    SetHitInfoForReplication(DamageContext);

    if (GetHealth() <= 0.0f)
    {
        HandleDeath(InstigatorActor, TargetActor, DamageEventData);
    }

}

void UAgoraAttributeSetHealth::HandleDeath(AActor* Killer, AActor* Killed, FGameplayEventData& DamageGameplayEvent)
{
    DamageGameplayEvent.EventTag = UAgoraGameplayTags::RequestDeath();
    int32 NumAbilitiesActivated = UAgoraBlueprintFunctionLibrary::SendGameplayEventToActor(Killed, UAgoraGameplayTags::RequestDeath(), DamageGameplayEvent);
    FDamageContext DeathDamageContext = FDamageContext(DamageGameplayEvent);

    // Here we died
    if (NumAbilitiesActivated == 0)
    {

        DamageGameplayEvent.EventTag = UAgoraGameplayTags::Death();
        UAgoraBlueprintFunctionLibrary::SendGameplayEventToActor(Killed, UAgoraGameplayTags::Death(), DamageGameplayEvent);
        APlayerState* KilledPlayerState = GetPlayerState(Killed);
        if (KilledPlayerState)
        {
            UAgoraStatTrackingLibrary::ModifyPlayerStat(KilledPlayerState, "Deaths", EStatModType::Add, 1.0f, true);
        }

        DamageGameplayEvent.EventTag = UAgoraGameplayTags::KilledActor();
        UAgoraBlueprintFunctionLibrary::SendGameplayEventToActor(Killer, UAgoraGameplayTags::KilledActor(), DamageGameplayEvent);
        APlayerState* KillerPlayerState = GetPlayerState(Killer);
        if (KillerPlayerState)
        {
            if (UAgoraGameplayTags::IsMinion(Killed))
            {
                UAgoraStatTrackingLibrary::ModifyPlayerStat(KillerPlayerState, "Kills.Minions", EStatModType::Add, 1.0f, true);
            }

            if (UAgoraGameplayTags::IsTower(Killed))
            {
                UAgoraStatTrackingLibrary::ModifyPlayerStat(KillerPlayerState, "Kills.Towers", EStatModType::Add, 1.0f, true);
            }

            if (UAgoraGameplayTags::IsHero(Killed))
            {
                UAgoraStatTrackingLibrary::ModifyPlayerStat(KillerPlayerState, "Kills.Hero", EStatModType::Add, 1.0f, true);
            }
        }
        
        TScriptInterface<IDamageReceiverInterface> DamageReceiver = GetOwningActor();
        if (DamageReceiver)
        {
            DamageReceiver->NotifyDied(DeathDamageContext);
        }

        AAgoraGameMode* Gamemode = GetWorld()->GetAuthGameMode<AAgoraGameMode>();
		if (Gamemode) {
			Gamemode->NotifyKilled(Killer, Killed, DeathDamageContext);
		}

		TRACE(AgoraCharacter, Log, "%s died. Context: %s", *GetNameSafe(GetOwningActor()), *DeathDamageContext.ToString())
    }

    // Here we prevented death
    TRACE(AgoraCharacter, Log, "%s Prevented Death. Context: %s", *GetNameSafe(GetOwningActor()), *DeathDamageContext.ToString())
}

void UAgoraAttributeSetHealth::SetHitInfoForReplication(const FDamageContext& DamageContext)
{
    LastTakeHitInfo.DamageContext = DamageContext;
    LastTakeHitInfo.Time = UAgoraBlueprintFunctionLibrary::GetTimeSinceStartOfGame(GetOwningActor());
    OnRep_LastTakeHitInfo();
}

APlayerState* UAgoraAttributeSetHealth::GetPlayerState(AActor* Actor)
{
    APawn* ActorAsPawn = Cast<APawn>(Actor);
    if (ActorAsPawn)
    {
        return ActorAsPawn->GetPlayerState();
    }
    return nullptr;
}

void UAgoraAttributeSetHealth::OnRep_LastTakeHitInfo()
{
    if (!LastTakeHitInfo.IsStale(UAgoraBlueprintFunctionLibrary::GetTimeSinceStartOfGame(GetOwningActor())))
    {
        TScriptInterface<IDamageDealerInterface> DamageDealer = LastTakeHitInfo.DamageContext.DamageInstigator;
        TScriptInterface<IDamageReceiverInterface> DamageReceiver = LastTakeHitInfo.DamageContext.DamagedActor;

		if (DamageDealer && DamageReceiver)
		{
			DamageReceiver->NotifyDamageReceived(GetHealth(),GetMaxHealth(),LastTakeHitInfo.DamageContext);
			DamageDealer->NotifyDamageInstigated(LastTakeHitInfo.DamageContext);
		}
    }
}

void UAgoraAttributeSetHealth::OnRep_Health()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetHealth, Health);
}

void UAgoraAttributeSetHealth::OnRep_MaxHealth()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetHealth, MaxHealth);
}

void UAgoraAttributeSetHealth::OnRep_HealthRegen()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetHealth, HealthRegen);
}

void UAgoraAttributeSetHealth::OnRep_PhysicalDefense()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetHealth, PhysicalDefense);
}

void UAgoraAttributeSetHealth::OnRep_MagicalDefense()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetHealth, MagicalDefense);
}



