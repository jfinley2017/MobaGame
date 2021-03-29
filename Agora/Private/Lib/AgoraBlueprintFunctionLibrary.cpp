// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.
#include "AgoraBlueprintFunctionLibrary.h"
#include "Runtime/CoreUObject/Public/UObject/Interface.h"
#include "EngineMinimal.h"
#include "AIModule/Classes/BrainComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIModule/Classes/AIController.h"
#include "Matrix.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Landscape.h"
#include "Components/SkeletalMeshComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "DrawDebugHelpers.h"
#include "Math/NumericLimits.h"
#include "AbilitySystemGlobals.h" 
#include "GameplayCueManager.h" 
#include "GameplayAbilities/Public/AbilitySystemBlueprintLibrary.h" 
#include "Engine/NetConnection.h"
#include "Kismet/GameplayStatics.h"

#include "Agora.h"
#include "AgoraCharacterBase.h"
#include "GASCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AgoraAbilitySystemComponent.h"
#include "Lib/AgoraDataSingleton.h"
#include "AgoraGameplayTags.h"
#include "Lib/AgoraWorldSettings.h"
#include "AgoraTeamLibrary.h"
#include "AgoraVisionLibrary.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Texture.h"
#include "Modes/AgoraGameState.h"
#include "AgoraTagLibrary.h"

FGameplayCueParameters UAgoraBlueprintFunctionLibrary::MakeTargetedGameplayCueParams(UAbilitySystemComponent* AbilitySystemComp, AActor* Target, AActor* Instigator, FVector Origin)
{

	FGameplayCueParameters CueParams;
	FGameplayEffectContextHandle CueContext = AbilitySystemComp->MakeEffectContext();

	TWeakObjectPtr<AActor> TargetPtr = Cast<AActor>(Target);

	TArray<TWeakObjectPtr<AActor>> TargetActors;
	TargetActors.Add(TargetPtr);

	CueContext.AddActors(TargetActors);
	CueContext.AddInstigator(Instigator, Instigator);
	CueContext.AddSourceObject(Instigator);
	CueContext.AddOrigin(Origin);

	CueParams.EffectContext = CueContext;
	return CueParams;
}

void UAgoraBlueprintFunctionLibrary::ImplementsInterfaceLogging(UObject* TestActor, TSubclassOf<UInterface> Interface, bool bShouldLogFailure, bool bShouldEnsureTrue, TEnumAsByte<EClassCheckEnum>& Branches)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) // Do not Print in Shipping or Test
    if (TestActor && TestActor->GetClass()->ImplementsInterface(Interface))
    {
        Branches = EClassCheckEnum::OnSuccess;
    }
    else
    {
        if (bShouldLogFailure)
        {
            if (bShouldEnsureTrue)
            {
                // Crash.
                TRACESTATIC(Agora, Fatal, "%s tried to be a(n) %s. They failed.", *GetNameSafe(TestActor), *Interface->GetName());

            }
            // Just error
            TRACESTATIC(Agora, Error, "%s tried to be a(n) %s. They failed.", *GetNameSafe(TestActor), *Interface->GetName());
        }
        // In reality this type of error should probably cause a crash in most situations, but maybe there still are some which should allow the engine to live
        if (bShouldEnsureTrue)
        {
            ensure(false);
        }
        Branches = EClassCheckEnum::OnFailure;
    }
#endif
}

void UAgoraBlueprintFunctionLibrary::IsALogging(UObject* TestActor, UClass* Other, bool bShouldLogFailure, bool bShouldEnsureTrue, TEnumAsByte<EClassCheckEnum>& Branches)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) // Do not Print in Shipping or Test

    if (TestActor && TestActor->GetClass()->IsChildOf(Other))
    {
        Branches = EClassCheckEnum::OnSuccess;
    }
    else
    {
        if (bShouldLogFailure)
        {
            if (bShouldEnsureTrue)
            {
                // Crash.
                TRACESTATIC(Agora, Fatal, "%s tried to be a(n) %s. They failed.", *GetNameSafe(TestActor), *Other->GetName());

            }
            // Just error
            TRACESTATIC(Agora, Error, "%s tried to be a(n) %s. They failed.", *GetNameSafe(TestActor), *Other->GetName());
        }
        if (bShouldEnsureTrue)
        {
            // In reality this type of error should proooooooobably cause a crash in most situations, but maybe there still are some which should allow the engine to live
            ensure(false);
        }
        Branches = EClassCheckEnum::OnFailure;
    }
#endif

}

EDamageType UAgoraBlueprintFunctionLibrary::DetermineDamageTypeFromContextHandle(const FGameplayEffectContextHandle& InHandle)
{
    FGameplayTagContainer HandleTagContainer;
    UAgoraBlueprintFunctionLibrary::GetGameplayEffectContextTags(InHandle, HandleTagContainer);

    if (HandleTagContainer.HasTag(UAgoraGameplayTags::DamageTypeMagical()))
    {
        return EDamageType::DMGMagical;
    }
    else if (HandleTagContainer.HasTag(UAgoraGameplayTags::DamageTypePhysical()))
    {
        return EDamageType::DMGPhysical;
    }
    else if (HandleTagContainer.HasTag(UAgoraGameplayTags::DamageTypeTrue()))
    {
        return EDamageType::DMGTrue;
    }

    return EDamageType::DMGInvalid;
}

FName UAgoraBlueprintFunctionLibrary::DetermineHitDirection(const FVector& RightVector, const FVector& HitResultNormal)
{
    FName OutHitDirection = NAME_None;
    float DotProduct = FVector::DotProduct(RightVector, HitResultNormal);

    if (DotProduct >= 0.7) // Hit from front
    {
        
        OutHitDirection = FName("Front");
    }
    else if (DotProduct <= -0.7) // Hit from back
    {
        OutHitDirection = FName("Back");
    }
    else
    {
        if ((RightVector * HitResultNormal).X > 0) // Hit from left
        {
            OutHitDirection = FName("Left");
        }
        else // Hit from right
        {
            OutHitDirection = FName("Right");
        }
    }
    return OutHitDirection;
}

UAgoraUserSettings* UAgoraBlueprintFunctionLibrary::GetAgoraUserSettings()
{
	return UAgoraUserSettings::Get();
}

bool UAgoraBlueprintFunctionLibrary::GetPlayerNetConnectionDetails(const APlayerController* PlayerController, FAgoraNetConnection& OutNetConnection)
{
    UNetConnection* NetCon = PlayerController->GetNetConnection();
    APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>();

    if (!NetCon) { return false; }

    OutNetConnection.FrameTime = NetCon->FrameTime;
    OutNetConnection.Ping = PlayerState ? PlayerState->ExactPingV2 : 0.0f;
    OutNetConnection.PktLoss = NetCon->InPacketsLost > 0 ? (NetCon->InPackets / NetCon->InPacketsLost) * 100 : 0.0f;
    return true;

}

bool UAgoraBlueprintFunctionLibrary::IsRunningWithEditor()
{
#if WITH_EDITOR
    return true;
#endif
    return false;
}

APlayerController* UAgoraBlueprintFunctionLibrary::GetViewingPlayerController(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

    if (WorldContextObject->GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer) { return nullptr; }
    return WorldContextObject->GetWorld()->GetFirstPlayerController();
}

AAgoraWorldSettings* UAgoraBlueprintFunctionLibrary::GetAgoraWorldSettings(UObject* WorldContextObject)
{
	return Cast<AAgoraWorldSettings>(WorldContextObject->GetWorld()->GetWorldSettings());
}

UAgoraDataSingleton* UAgoraBlueprintFunctionLibrary::GetGlobals()
{
    return Cast<UAgoraDataSingleton>(GEngine->GameSingleton);
}

UObject* UAgoraBlueprintFunctionLibrary::BP_GetDefaultObject(UClass* ObjClass)
{
	if (!ObjClass) { return nullptr; }

	return ObjClass->GetDefaultObject();
}

bool UAgoraBlueprintFunctionLibrary::ShouldPlayCosmetics(UObject* WorldContextObject)
{
    return WorldContextObject->GetWorld()->GetNetMode() != ENetMode::NM_DedicatedServer;
}

FGameplayAttribute UAgoraBlueprintFunctionLibrary::GetAttributeFromString(const FString& AttributeName, UAbilitySystemComponent* ASC)
{
	TArray<FGameplayAttribute> Attributes;
	ASC->GetAllAttributes(Attributes);

	for (FGameplayAttribute Attr : Attributes)
	{
		if (Attr.AttributeName == AttributeName)
		{
			return Attr;
		}
	}

	return FGameplayAttribute();
}

float UAgoraBlueprintFunctionLibrary::GetTimeSinceStartOfGame(UObject* WorldContextObject)
{
    if (!WorldContextObject) 
    {
        TRACESTATIC(Agora, Fatal, "Invalid WorldContext")
    }

	return Cast<AAgoraGameState>(UGameplayStatics::GetGameState(WorldContextObject))->GetSecondsSinceMatchStart();
}

AAgoraWorldSettings* UAgoraBlueprintFunctionLibrary::GetWorldSettings(UObject* WorldContextObject)
{
	return Cast<AAgoraWorldSettings>(WorldContextObject->GetWorld()->GetWorldSettings());
}

void UAgoraBlueprintFunctionLibrary::GetAllPawns(UObject* WorldContextObject, TArray<APawn*>& OutPawns, AActor* ReferenceActor, const FAgoraSweepQueryParams& QueryParams)
{
    if (!WorldContextObject || !WorldContextObject->ImplementsGetWorld()) 
    { 
        TRACESTATIC(Agora, Error, "WorldContextObject: %s doesn't exist or is invalid.", *GetNameSafe(WorldContextObject)); 
        return;
    }

    for (FConstPawnIterator Iterator = WorldContextObject->GetWorld()->GetPawnIterator(); Iterator; Iterator++)
    {   
        if (QueryParams.MatchesQuery(ReferenceActor, Iterator->Get()))
        {
            OutPawns.Push(Iterator->Get());
        }
    }

}

float UAgoraBlueprintFunctionLibrary::CalculateMontagePlayRate(UAnimMontage* MontageToScale, float DesiredPlayLength, EClampType ClampMode, float MinPlayRate, float MaxPlayRate)
{
    if (!MontageToScale) { return 1.0f; }

    float ScaledPlayRate = MontageToScale->GetPlayLength() / DesiredPlayLength;

    switch (ClampMode)
    {
    case EClampType::ClampBoth:
        return FMath::Clamp(ScaledPlayRate, MinPlayRate, MaxPlayRate);
    case EClampType::ClampMax:
        return FMath::Clamp(ScaledPlayRate, 0.0f, MaxPlayRate);
    case EClampType::ClampMin:
        return FMath::Clamp(ScaledPlayRate, MinPlayRate, ScaledPlayRate);
    default:
        return FMath::Max(0.0f, ScaledPlayRate);
    }

}

FHitResult UAgoraBlueprintFunctionLibrary::DrawTraceFromActor(AActor* InSourceActor, float MaxRange, bool bDebug)
{
	FHitResult ReturnHitResult;
	
	FRotator LookDirection = FRotator::ZeroRotator;
	FVector LookLocation = FVector::ZeroVector;
	InSourceActor->GetActorEyesViewPoint(LookLocation, LookDirection); // get the location and direction of the camera
	
	FVector TraceStart = LookLocation;
	FVector TraceEnd = LookDirection.Vector() * 10000 + LookLocation;

	FCollisionQueryParams LineTraceParams;
	LineTraceParams.AddIgnoredActor(InSourceActor);

	//Use a line trace initially to see where the player is actually pointing
	InSourceActor->GetWorld()->LineTraceSingleByChannel(ReturnHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, LineTraceParams);
	if (bDebug)
	{
		DrawDebugLine(InSourceActor->GetWorld(), TraceStart, TraceEnd, FColor::Red, true, 5.0f);
	}

	//Default to end of trace line if we don't hit anything.
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
	}
	TraceStart = InSourceActor->GetActorLocation();
	FVector DirectionVector = (ReturnHitResult.Location - TraceStart).GetSafeNormal();
	TraceEnd = TraceStart + DirectionVector * MaxRange;

	//Second trace, from actor towards ViewPoint
	InSourceActor->GetWorld()->LineTraceSingleByChannel(ReturnHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, LineTraceParams);
	if (bDebug)
	{
		DrawDebugLine(InSourceActor->GetWorld(), TraceStart, TraceEnd, FColor::Green, true, 5.0f, 0, 2.0f);
	}

	return ReturnHitResult;
}

float UAgoraBlueprintFunctionLibrary::GetSectionLengthFromIndex(UAnimMontage* Montage, int32 SectionIndex)
{
    return Montage ? Montage->GetSectionLength(SectionIndex) : 0.0f;
}

float UAgoraBlueprintFunctionLibrary::GetSectionLengthFromName(UAnimMontage* Montage, FName SectionName)
{
    if (!Montage) { return 0.0f; }
    return SectionName == NAME_None ? Montage->GetSectionLength(0) : Montage->GetSectionLength(Montage->GetSectionIndex(SectionName));
}

float UAgoraBlueprintFunctionLibrary::GenerateRandomFloat(float Min, float Max)
{
    return FMath::FRandRange(Min, Max);
}

FString UAgoraBlueprintFunctionLibrary::CreateOptionsStringForOpenLevel(const TMap<FString, FString>& Options)
{
	FString OptionsString = "";
	for (const TPair<FString, FString>& Option : Options)
	{
		OptionsString.AppendChar('?');
		OptionsString.Append(Option.Key);
		OptionsString.AppendChar('=');
		OptionsString.Append(Option.Value);
	}
	return OptionsString;
}

EPhysicalSurface UAgoraBlueprintFunctionLibrary::GetStaticMeshSurfaceTypeFromHitResult(const FHitResult& HitResult)
{
	AStaticMeshActor* Mesh = Cast<AStaticMeshActor>(HitResult.GetActor());
	if (Mesh)
	{
		//MWAHAHAHAHAHAHA THIS WAS SUPPOSED TO BE HIDDEN FROM ME
		//OVER MY DEAD BODY TIM
		UPhysicalMaterial* Mat = Mesh->GetStaticMeshComponent()->BodyInstance.GetSimplePhysicalMaterial(); 
		return Mat->SurfaceType;
	}
	return EPhysicalSurface::SurfaceType_Default;
}

EPhysicalSurface UAgoraBlueprintFunctionLibrary::GetLandscapeSurfaceTypeFromHitResult(const FHitResult& HitResult)
{
	ALandscape* Landscape = Cast<ALandscape>(HitResult.Actor.Get());
	if (Landscape && Landscape->DefaultPhysMaterial)
	{
		return Landscape->DefaultPhysMaterial->SurfaceType;
	}
	return EPhysicalSurface::SurfaceType_Default;
}

void UAgoraBlueprintFunctionLibrary::UpdateActorOverlaps(AActor* Actor)
{
	Actor->UpdateOverlaps(true);
}

void UAgoraBlueprintFunctionLibrary::UpdateComponentOverlaps(UPrimitiveComponent* Component)
{
	Component->UpdateOverlaps();
}

FVector UAgoraBlueprintFunctionLibrary::GetPlayerLookingPoint(AAgoraCharacterBase* Player, float Range)
{
	FVector ViewPoint;
	FRotator ViewRotation;
	Player->GetController()->GetPlayerViewPoint(ViewPoint, ViewRotation);
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(Player->GetUniqueID());
	FVector LookAtPoint;
	bool TryTrace = Player->GetWorld()->LineTraceSingleByChannel(HitResult, ViewPoint, ViewPoint + ViewRotation.Vector() * Range, ECC_Visibility, QueryParams);
	if (TryTrace)
	{
		return HitResult.ImpactPoint;
	}
	else
	{
		return FVector();
	}
}

void UAgoraBlueprintFunctionLibrary::GetPlayerViewLocationAndRotation(AActor* Player, FVector& OutLocation, FRotator& OutRotation)
{
	ACharacter* PlayerPawn = Cast<ACharacter>(Player);
	if (PlayerPawn)
	{
		PlayerPawn->GetController()->GetPlayerViewPoint(OutLocation, OutRotation);
	}

	Player->GetActorEyesViewPoint(OutLocation, OutRotation);
}

void UAgoraBlueprintFunctionLibrary::SaveTextFile(const FString& FileName, const FString& FileContents)
{
	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();
	FString SavedDir = FPaths::ProjectSavedDir();
	if (File.CreateDirectoryTree(*SavedDir))
	{
		FString AbsolutePath = SavedDir + "/" + FileName;
		FFileHelper::SaveStringToFile(FileContents, *AbsolutePath);
	}
}

bool UAgoraBlueprintFunctionLibrary::LoadTextFile(const FString& FileName, FString& OutContents)
{
	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();
	FString SavedDir = FPaths::ProjectSavedDir();
	FString AbsolutePath = SavedDir + "/" + FileName;
	return FFileHelper::LoadFileToString(OutContents, *FileName);
}

bool UAgoraBlueprintFunctionLibrary::DeleteTextFile(const FString& FileName)
{
	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();
	FString SavedDir = FPaths::ProjectSavedDir();
	FString AbsolutePath = SavedDir + "/" + FileName;
	return File.DeleteFile(*AbsolutePath);
}

void UAgoraBlueprintFunctionLibrary::AddImpulseToCharacterWithMagnitudes(AAgoraCharacterBase* Character, float UpMagnitude, float ForwardMagnitude)
{
	FVector Impulse = GetVectorPointingFromActor(Character, UpMagnitude, ForwardMagnitude);
	Character->GetCharacterMovement()->AddImpulse(Impulse, true);
}

void UAgoraBlueprintFunctionLibrary::ImpulseActors(const TArray<AActor*>& Actors, const FVector& Impulse)
{
	for (AActor* Actor : Actors)
	{
		Cast<AAgoraCharacterBase>(Actor)->GetGASCMC()->AddImpulse(Impulse, true);
	}
}

void UAgoraBlueprintFunctionLibrary::AddImpulseToCharacterWithVector(AAgoraCharacterBase* Character, FVector Impulse)
{
	Character->GetCharacterMovement()->AddImpulse(Impulse, true);
}

void UAgoraBlueprintFunctionLibrary::DisableGravityForCharacter(AAgoraCharacterBase* Character)
{
	Character->GetCharacterMovement()->GravityScale = 0.0f;
	Character->GetCharacterMovement()->StopMovementImmediately();
}

void UAgoraBlueprintFunctionLibrary::EnableGravityForCharacter(AAgoraCharacterBase* Character)
{
	Character->GetCharacterMovement()->GravityScale = 1.0f;
}

FGameplayTagContainer UAgoraBlueprintFunctionLibrary::GetSourceAbilityTags(const FGameplayEffectContextHandle& InHandle)
{
    return InHandle.IsValid() && InHandle.GetAbility() ? InHandle.GetAbility()->AbilityTags : FGameplayTagContainer();
}

AActor* UAgoraBlueprintFunctionLibrary::GetEffectCauser(const FGameplayEffectContextHandle& InHandle)
{
    return InHandle.IsValid() ? InHandle.GetEffectCauser() : nullptr;
}

void UAgoraBlueprintFunctionLibrary::ApplySetByCallerGameplayEffectToTarget(UAbilitySystemComponent* TargetComp, int32 Level, TSubclassOf<UGameplayEffect> GameplayEffectClass, TArray<FSetByCallerData> SetByCallerData)
 {
	// Create and fill the spec with GE Data
	FGameplayEffectSpecHandle GESpecHandle;
	FGameplayEffectContextHandle ContextHandle = FGameplayEffectContextHandle();
	GESpecHandle = TargetComp->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);

	FGameplayEffectSpec* Spec = GESpecHandle.Data.Get();
	if (Spec && TargetComp)
	{
		for (FSetByCallerData Data : SetByCallerData)
		{
			Spec->SetSetByCallerMagnitude(Data.DataTag, Data.Magnitude);
		}

		TargetComp->ApplyGameplayEffectSpecToTarget(*Spec, TargetComp);
	}
}

FString UAgoraBlueprintFunctionLibrary::ArrayOfTagsToString(TArray<FGameplayTag> Tags)
{
    return UAgoraTagLibrary::ArrayOfTagsToString(Tags);
}

FVector UAgoraBlueprintFunctionLibrary::GetPositionInFrontOfActor(AActor* Actor, float DistanceInFront)
{
	FVector Forward = Actor->GetActorForwardVector();
	FVector Location = Actor->GetActorLocation();

	return (Forward * DistanceInFront) + Location;
}

FVector UAgoraBlueprintFunctionLibrary::GetPositionAboveActor(AActor* Actor, float DistanceAbove)
{
	FVector Up = Actor->GetActorUpVector();
	FVector Location = Actor->GetActorLocation();

	return (Up * DistanceAbove) + Location;
}

FVector UAgoraBlueprintFunctionLibrary::GetVectorPointingFromActor(AActor* Actor, float UpMagnitude, float ForwardMagnitude)
{
	FVector Forward = Actor->GetActorForwardVector();
	FVector Up = Actor->GetActorUpVector();

	return (Up * UpMagnitude) + (Forward * ForwardMagnitude);
}

FVector UAgoraBlueprintFunctionLibrary::ScaleVectorToLength(FVector Vector, float Length)
{
	Vector.Normalize();
	return Vector * Length;
}

UAgoraSceneTargetComponent* UAgoraBlueprintFunctionLibrary::GetSceneComponentFromFirstActorInTargetData(FGameplayAbilityTargetDataHandle TargetData)
{
	TArray<TWeakObjectPtr<AActor>> Actors = TargetData.Data.GetData()->Get()->GetActors();
    if (Actors.Num() > 0)
    {
        AAgoraCharacterBase* TargetAsCharacter = Cast<AAgoraCharacterBase>(Actors[0]);
        if (TargetAsCharacter)
        {
            return TargetAsCharacter->GetTargetComponent();
        }
		return NULL;
		
	}
	return NULL;
}

int32 UAgoraBlueprintFunctionLibrary::SendGameplayEventToActor(AActor* Actor, const FGameplayTag EventTag, const FGameplayEventData Payload)
{
    UAbilitySystemComponent* ActorAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
    if (!ActorAbilitySystemComponent)
    {
		TRACESTATIC(Agora, Warning, "%s is unable to receive GameplayEvent. Failed to retrieve AbilitySystemComponent", *GetNameSafe(Actor))
		return 0;
    }
    return ActorAbilitySystemComponent->HandleGameplayEvent(EventTag, &Payload);
}

void UAgoraBlueprintFunctionLibrary::GetGameplayEffectContextTags(const FGameplayEffectContextHandle& InHandle, FGameplayTagContainer& OutContainer)
{
    FAgoraGameplayEffectContext* DamageContext = (FAgoraGameplayEffectContext*)InHandle.Get();
    if(!DamageContext) 
    {
        TRACESTATIC(Agora, Fatal, "%s could not be converted into FGameplayEffectDamageContext.",*InHandle.ToString()) 
    }

    DamageContext->GetContextTags(OutContainer);
    
}

void UAgoraBlueprintFunctionLibrary::PlayLocalGameplayCueForActor(AActor* CueTarget, FGameplayTag CueTag, const FGameplayCueParameters& EventData)
{

    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
        CueTarget,
        CueTag,
        EGameplayCueEvent::Executed,
        EventData);

}

//////////////////////////////////////////////////////////////////////////
// Deprecated

bool UAgoraBlueprintFunctionLibrary::IsFriendly(const AActor* Actor1, const AActor* Actor2)
{
    return UAgoraTeamLibrary::IsFriendly(Actor1, Actor2);
}

bool UAgoraBlueprintFunctionLibrary::HasVisionOf(AActor* ActorOne, AActor* ActorTwo)
{
    return UAgoraVisionLibrary::HasVisionOf(ActorOne, ActorOne, ActorTwo);
}

bool UAgoraBlueprintFunctionLibrary::AgoraSweepMultiSphere(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, float SweepSphereRadius, const FVector& SweepLocation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors)
{
    return UAgoraSweepLibrary::AgoraSweepMultiSphere(WorldContextObject, SweepingActor, AgoraQueryParams, SweepSphereRadius, SweepLocation, OutOverlappingActors, IgnoredActors);
}

bool UAgoraBlueprintFunctionLibrary::AgoraSweepMultiBox(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, const FVector& SweepBoxExtent, const FVector& SweepLocation, const FRotator& SweepRotation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors)
{
    return UAgoraSweepLibrary::AgoraSweepMultiBox(WorldContextObject, SweepingActor, AgoraQueryParams, SweepBoxExtent, SweepLocation, SweepRotation, OutOverlappingActors, IgnoredActors);
}

bool UAgoraBlueprintFunctionLibrary::AgoraSweepMultiCapsule(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams, const float SweepCapsuleRadius, const float SweepCapsuleHalfHeight, const FVector& SweepLocation, const FRotator& SweepRotation, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors)
{
    return UAgoraSweepLibrary::AgoraSweepMultiCapsule(WorldContextObject, SweepingActor, AgoraQueryParams, SweepCapsuleRadius, SweepCapsuleHalfHeight, SweepLocation, SweepRotation, OutOverlappingActors, IgnoredActors);
}

bool UAgoraBlueprintFunctionLibrary::AgoraSweepMultiPyramid(UObject* WorldContextObject, AActor* SweepingActor, const FAgoraSweepQueryParams& AgoraQueryParams,
                                                            const float BaseWidth, const float BaseHeight, const FVector& PyramidOrigin, const float PyramidHeight, const FRotator& PyramidRotation,
                                                            TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& IgnoredActors, const FVector2D& Spread, bool bDebug)
{
    return UAgoraSweepLibrary::AgoraSweepMultiPyramid(WorldContextObject, SweepingActor, AgoraQueryParams, BaseWidth, BaseHeight, PyramidOrigin, PyramidHeight, PyramidRotation, OutOverlappingActors, IgnoredActors, Spread, bDebug);
}

USkeletalMeshComponent* UAgoraBlueprintFunctionLibrary::GetMeshFromActor(AActor*Actor)
{
    return UAgoraEntityLibrary::GetMesh(Actor);
}

EDescriptor UAgoraBlueprintFunctionLibrary::GetDescriptor(AActor* Actor)
{
    return UAgoraEntityLibrary::GetEntityType(Actor);
}

FGameplayTag UAgoraBlueprintFunctionLibrary::GetGameplayTag(const FName Tag)
{
    return UAgoraTagLibrary::GetGameplayTag(Tag);
}

bool UAgoraBlueprintFunctionLibrary::HasMatchingGameplayTag(AActor* Actor, const FGameplayTag Tag)
{
    return UAgoraTagLibrary::HasMatchingGameplayTag(Actor, Tag);
}

bool UAgoraBlueprintFunctionLibrary::HasAllMatchingGameplayTags(AActor* Actor, const FGameplayTagContainer& TagContainer)
{
    return UAgoraTagLibrary::HasAllMatchingGameplayTags(Actor, TagContainer);
}

bool UAgoraBlueprintFunctionLibrary::HasAnyMatchingGameplaytags(AActor* Actor, const FGameplayTagContainer& TagContainer)
{
    return UAgoraTagLibrary::HasAnyMatchingGameplayTags(Actor, TagContainer);
}

void UAgoraBlueprintFunctionLibrary::GetOwnedGameplayTags(AActor* Actor, FGameplayTagContainer& OutContainer)
{
    return UAgoraTagLibrary::GetOwnedGameplayTags(Actor, OutContainer);
}

FMeleeHitResult UAgoraBlueprintFunctionLibrary::GenerateMeleeHitResult_Cleave(AActor* PerformingActor, FVector HitBox, float MaxRange, bool bDebug)
{
    return UAgoraSweepLibrary::GenerateMeleeHitResult_Cleave(PerformingActor, HitBox, MaxRange, bDebug);
}

UTexture* UAgoraBlueprintFunctionLibrary::GetDisplayIcon(AActor* Actor)
{
    return UAgoraEntityLibrary::GetDisplayIcon(Actor);
}

void UAgoraBlueprintFunctionLibrary::CompletelyDisableInput(APawn* PawnToDisable)
{
    UAgoraEntityLibrary::CompletelyDisableInput(PawnToDisable);
}

void UAgoraBlueprintFunctionLibrary::EnableInput(APawn* PawnToEnable)
{
    UAgoraEntityLibrary::EnableInput(PawnToEnable);
}

void UAgoraBlueprintFunctionLibrary::DisableMovement(APawn* PawnToDisable)
{
    UAgoraEntityLibrary::DisableMovement(PawnToDisable);
}

void UAgoraBlueprintFunctionLibrary::EnableMovement(APawn* PawnToDisable)
{
    UAgoraEntityLibrary::EnableMovement(PawnToDisable);
}

void UAgoraBlueprintFunctionLibrary::EnableMouseLook(APawn* PawnToEnable)
{
    UAgoraEntityLibrary::EnableInput(PawnToEnable);
}

void UAgoraBlueprintFunctionLibrary::DisableMouseLook(APawn* PawnToDisable)
{
    UAgoraEntityLibrary::DisableMouseLook(PawnToDisable);
}

void UAgoraBlueprintFunctionLibrary::DisableTurnInPlace(AAgoraCharacterBase* Character)
{
    UAgoraEntityLibrary::DisableTurnInPlace(Character);
}

void UAgoraBlueprintFunctionLibrary::EnableTurnInPlace(AAgoraCharacterBase* Character)
{
    UAgoraEntityLibrary::EnableTurnInPlace(Character);
}