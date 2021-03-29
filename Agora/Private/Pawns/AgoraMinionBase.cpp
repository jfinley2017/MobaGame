#include "AgoraMinionBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"

#include "Agora.h"
#include "AgoraLaneMinionAIController.h"
#include "AgoraBlueprintFunctionLibrary.h"

AAgoraMinionBase::AAgoraMinionBase(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
	SetReplicates(true);
	SetReplicateMovement(true);
    GetMesh()->SetCullDistance(7500.0f);
	GetCharacterMovement()->bUseRVOAvoidance = true;
    
}

void AAgoraMinionBase::FinishPlayDying()
{   
    SetActorHiddenInGame(true);
}

void AAgoraMinionBase::FinishDying()
{
    Destroy();
}

void AAgoraMinionBase::Attack()
{
    // c++ stuff
   
    // blueprint stuff
    OnAttack();
}

bool AAgoraMinionBase::IsDead() const
{
    return bIsDead;
}

void AAgoraMinionBase::NotifyDied(const FDamageContext& DamageContext)
{
    Super::NotifyDied(DamageContext);
    bIsDead = true;
}

