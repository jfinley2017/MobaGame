// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraMapIconComponent.h"
#include "Agora.h"
#include "AgoraMapTrackerService.h"
#include "AgoraMapLibrary.h"
#include "AgoraAffiliatedWidget.h"
#include "UserWidget.h"
#include "AgoraBlueprintFunctionLibrary.h"
#include "AgoraVisionLibrary.h"

// Sets default values for this component's properties
UAgoraMapIconComponent::UAgoraMapIconComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAgoraMapIconComponent::BeginPlay()
{
	Super::BeginPlay();

    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    if (WidgetClass)
    {
		
        CachedWidget = CreateWidget<UAgoraAffiliatedWidget>(GetWorld(), WidgetClass);
		CachedWidget->SetObserved(GetOwner());
    }

    CachedMapTrackerService = UAgoraMapLibrary::GetMapTrackerService(GetWorld());
   
    static bool InvalidMapServiceWarning = true;
    if (!CachedMapTrackerService)
    {
        if (InvalidMapServiceWarning)
        {
            TRACE(AgoraMap, Error, "No MapTrackerService available. AgoraMapIconComponents disabled.");
            InvalidMapServiceWarning = false;
        }
        return;
    }

    IDamageReceiverInterface* OwnerAsDamageReceiver = Cast<IDamageReceiverInterface>(GetOwner());
    if (OwnerAsDamageReceiver)
    {
        OwnerAsDamageReceiver->GetDeathDelegate().AddDynamic(this, &UAgoraMapIconComponent::NotifyOwnerDied);
    }

    AAgoraCharacterBase* OwnerAsCharacter = Cast<AAgoraCharacterBase>(GetOwner());
    if (OwnerAsCharacter)
    {
        OwnerAsCharacter->OnRespawned.AddDynamic(this, &UAgoraMapIconComponent::NotifyOwnerRespawned);
    }

    IVisionInterface* OwnerAsVisionInterface = Cast<IVisionInterface>(GetOwner());
    if (OwnerAsVisionInterface)
    {
        OwnerAsVisionInterface->GetBecameHiddenDelegate().AddDynamic(this, &UAgoraMapIconComponent::NotifyOwnerHidden);
        OwnerAsVisionInterface->GetBecameVisibleDelegate().AddDynamic(this, &UAgoraMapIconComponent::NotifyOwnerRevealed);
    }

    if (UAgoraVisionLibrary::LocalPlayerHasVisionOf(GetWorld(), GetOwner()))
    {
        CachedMapTrackerService->RegisterWithTracker(GetOwner());
    }

    if (DisplayRules == EMapIconDisplayRules::AlwaysVisibleRemoveOnDeath && CachedMapTrackerService)
    {
        CachedMapTrackerService->RegisterWithTracker(GetOwner());
    }

}

// Called every frame
void UAgoraMapIconComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

void UAgoraMapIconComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

    if (CachedMapTrackerService)
    {
        CachedMapTrackerService->UnRegisterWithTracker(GetOwner());
    }

    Super::EndPlay(EndPlayReason);

}

void UAgoraMapIconComponent::NotifyOwnerDied(const FDamageContext& KillingBlow)
{
    if (CachedMapTrackerService)
    {
        CachedMapTrackerService->UnRegisterWithTracker(GetOwner());
    }
}

void UAgoraMapIconComponent::NotifyOwnerRevealed()
{
    if (CachedMapTrackerService && ( DisplayRules != EMapIconDisplayRules::AlwaysVisibleRemoveOnDeath) )
    {
        CachedMapTrackerService->RegisterWithTracker(GetOwner());
    }
}

void UAgoraMapIconComponent::NotifyOwnerRespawned()
{
    if (UAgoraVisionLibrary::LocalPlayerHasVisionOf(GetWorld(),GetOwner()) && CachedMapTrackerService)
    {
        CachedMapTrackerService->RegisterWithTracker(GetOwner());
    }
}

void UAgoraMapIconComponent::NotifyOwnerHidden()
{
    if (CachedMapTrackerService && (DisplayRules != EMapIconDisplayRules::AlwaysVisibleRemoveOnDeath))
    {
        CachedMapTrackerService->UnRegisterWithTracker(GetOwner());
    }
}

UAgoraAffiliatedWidget* UAgoraMapIconComponent::GetIcon()
{
    return CachedWidget;
}

EMapIconEnum UAgoraMapIconComponent::GetDrawLevel()
{
    return DrawLevel;
}


