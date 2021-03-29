// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraTagLibrary.h"
#include "GameplayTagAssetInterface.h"
#include "Agora.h"

DEFINE_LOG_CATEGORY(AgoraTag);

IGameplayTagAssetInterface* UAgoraTagLibrary::GetAsTagAssetInterface(AActor* Actor)
{
    if (!Actor)
    {
        TRACESTATIC(AgoraTag, Warning, "Nullptr passed to HasMatchingGameplayTag");
        return nullptr;
    }

    IGameplayTagAssetInterface* ActorAsTagAssetInterface = Cast<IGameplayTagAssetInterface>(Actor);

    if (!ActorAsTagAssetInterface)
    {
        TRACESTATIC(AgoraTag, Warning, "IGameplayTagAssetInterface missing from %s", *GetNameSafe(Actor));
    }
    return ActorAsTagAssetInterface;
}

FGameplayTag UAgoraTagLibrary::GetGameplayTag(const FName Tag)
{
    FGameplayTag TagRequest = FGameplayTag::RequestGameplayTag(Tag);
    return TagRequest;
}

FString UAgoraTagLibrary::ArrayOfTagsToString(TArray<FGameplayTag> Tags)
{
    FString LogString = "Tags: [";
    for (FGameplayTag Tag : Tags)
    {
        LogString.Append(Tag.ToString()).Append(", ");
    }
    LogString.Append("]");
    return LogString;
}

bool UAgoraTagLibrary::HasMatchingGameplayTag(AActor* Actor, const FGameplayTag Tag)
{
    IGameplayTagAssetInterface* ActorAsTagInterface = GetAsTagAssetInterface(Actor);
    return ActorAsTagInterface ? ActorAsTagInterface->HasMatchingGameplayTag(Tag) : false;
}

bool UAgoraTagLibrary::HasAllMatchingGameplayTags(AActor* Actor, const FGameplayTagContainer& TagContainer)
{
    IGameplayTagAssetInterface* ActorAsTagInterface = GetAsTagAssetInterface(Actor);
    return ActorAsTagInterface ? ActorAsTagInterface->HasAllMatchingGameplayTags(TagContainer) : false;
}

bool UAgoraTagLibrary::HasAnyMatchingGameplayTags(AActor* Actor, const FGameplayTagContainer& TagContainer)
{
    IGameplayTagAssetInterface* ActorAsTagInterface = GetAsTagAssetInterface(Actor);
    return ActorAsTagInterface ? ActorAsTagInterface->HasAnyMatchingGameplayTags(TagContainer) : false;
}

void UAgoraTagLibrary::GetOwnedGameplayTags(AActor* Actor, FGameplayTagContainer& OutContainer)
{
    IGameplayTagAssetInterface* ActorAsTagInterface = GetAsTagAssetInterface(Actor);
    if (ActorAsTagInterface)
    {
        ActorAsTagInterface->GetOwnedGameplayTags(OutContainer);
    }
}
