// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.


#include "AgoraAttributeSetMovement.h"
#include "Net/UnrealNetwork.h"
#include "Agora.h"
#include "GameFramework/Character.h"
#include "UnrealMathUtility.h"
#include "AgoraEntityLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"


int32 AgoraMovementDisplayDirectionalPenalties = 0;

static FAutoConsoleVariableRef CVarAgoraMovementDirectionalPenalties(
    TEXT("Agora.MovementDirectionalPenalties"),
    AgoraMovementDisplayDirectionalPenalties,
    TEXT("Should we dump DirectionalMovement penalties to the screen?"));


UAgoraAttributeSetMovement::UAgoraAttributeSetMovement():
    MovementSpeedBase(0.0f),
    MovementSpeedBonus(0.0f),
    MovementSpeedSlow(0.0f)
{

}

void UAgoraAttributeSetMovement::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);
}

void UAgoraAttributeSetMovement::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data)
{
    Super::PostGameplayEffectExecute(Data);
}

void UAgoraAttributeSetMovement::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetMovement, MovementSpeedBase, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetMovement, MovementSpeedBonus, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAgoraAttributeSetMovement, MovementSpeedSlow, COND_None, REPNOTIFY_Always);
}

float UAgoraAttributeSetMovement::GetMaxSpeed() const
{
    return (MovementSpeedBase.GetCurrentValue() 
            * (1 + MovementSpeedBonus.GetCurrentValue()) 
            * (1 - DetermineCurrentMovespeedPenalty()));

}

void UAgoraAttributeSetMovement::OnRep_MovementSpeedBase()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetMovement, MovementSpeedBase);
}

void UAgoraAttributeSetMovement::OnRep_MovementSpeedBonus()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetMovement, MovementSpeedBonus);
}

void UAgoraAttributeSetMovement::OnRep_MovementSpeedSlow()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAgoraAttributeSetMovement, MovementSpeedSlow);
}

float UAgoraAttributeSetMovement::DetermineCurrentMovespeedPenalty() const
{
    return DetermineCurrentDirectionalMovespeedPenalty() > DetermineCurrentMovespeedSlowPenalty() 
        ? DetermineCurrentDirectionalMovespeedPenalty() : DetermineCurrentMovespeedSlowPenalty();
}

float UAgoraAttributeSetMovement::DetermineCurrentDirectionalMovespeedPenalty() const
{
    
    if (!CachedAvatarAsCharacter)
    {
        const_cast<UAgoraAttributeSetMovement*>(this)->CachedAvatarAsCharacter = 
            Cast<ACharacter>(GetOwningAbilitySystemComponent()->GetAvatarActor());

        if (!CachedAvatarAsCharacter)
        {
            return 0.0f;
        }
    }

    // Non-heroes don't get directional movement penalties
    if (UAgoraEntityLibrary::GetEntityType(CachedAvatarAsCharacter) != EDescriptor::Hero)
    {
        return 0.0f;
    }

    // It's possible that we want to do this via acceleration rather than velocity. Acceleration captures
    // the actual input for each frame, but isn't replicated. Velocity is replicated but is slow to update
    // for lower strafe penalties this is more or less unnoticeable.
    FVector Velocity = CachedAvatarAsCharacter->GetVelocity();
    Velocity.Normalize();

    FVector ForwardVector = CachedAvatarAsCharacter->GetActorForwardVector();
    FVector RightVector = CachedAvatarAsCharacter->GetActorRightVector();

    float ForwardDotProd = FVector::DotProduct(ForwardVector, Velocity);
    float RightDotProd = FVector::DotProduct(RightVector, Velocity);

   
    if (FMath::IsNearlyEqual(RightDotProd, 1.0f, .05f) || FMath::IsNearlyEqual(RightDotProd, -1.0f, .05f))
    {
        if (AgoraMovementDisplayDirectionalPenalties > 0)
        {
            SCREENDEBUG_WHITE(-1, "Forward: %f Right: %f Direction: Strafe. Penalty: %f", ForwardDotProd, RightDotProd, BackpedalPenaltyAMount);
        }
        return StrafePenaltyAmount;
    }
    
    if (ForwardDotProd < 0.05f)
    {
        if (AgoraMovementDisplayDirectionalPenalties > 0)
        {
            SCREENDEBUG_WHITE(-1, "Forward: %f Right: %f Direction: Backpedal. Penalty: %f", ForwardDotProd, RightDotProd, BackpedalPenaltyAMount);
        }
        return BackpedalPenaltyAMount;
    }

    if (AgoraMovementDisplayDirectionalPenalties > 0)
    {
        SCREENDEBUG_WHITE(-1, "Forward: %f Right: %f Direction: Forward. Penalty: %f", ForwardDotProd, RightDotProd, 0.0f);
    }
    return 0.0f;

   
}

float UAgoraAttributeSetMovement::DetermineCurrentMovespeedSlowPenalty() const
{
    return MovementSpeedSlow.GetCurrentValue();
}
