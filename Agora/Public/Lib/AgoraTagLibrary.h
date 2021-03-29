// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "AgoraTagLibrary.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(AgoraTag, Log, All);

class IGameplayTagAssetInterface;

/**
 * 
 */
UCLASS()
class AGORA_API UAgoraTagLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    static IGameplayTagAssetInterface* GetAsTagAssetInterface(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "Agora")
    static FGameplayTag GetGameplayTag(const FName Tag);

    UFUNCTION(BlueprintPure, Category = "Agora")
    static FString ArrayOfTagsToString(TArray<FGameplayTag> Tags);

    /*
     * Check if the asset has a gameplay tag that matches against the specified tag (expands to include parents of asset tags)
     * @Return false if actor doesn't implement IGameplayTagAssetInterface
     */
    UFUNCTION(BlueprintPure, Category = "AgoraTags")
    static bool HasMatchingGameplayTag(AActor* Actor, const FGameplayTag Tag);

    /* Check if the asset has gameplay tags that matches against all of the specified tags(expands to include parents of asset tags)
     * @Return false if actor doesn't implement IGameplayTagAssetInterface
     */
    UFUNCTION(BlueprintPure, Category = "AgoraTags")
    static bool HasAllMatchingGameplayTags(AActor* Actor, const FGameplayTagContainer& TagContainer);

    /* Check if the asset has gameplay tags that matches against any of the specified tags(expands to include parents of asset tags)
     * @Return false if actor doesn't implement IGameplayTagAssetInterface
     */
    UFUNCTION(BlueprintPure, Category = "AgoraTags")
    static bool HasAnyMatchingGameplayTags(AActor* Actor, const FGameplayTagContainer& TagContainer);

    /* Returns the actors owned GameplayTags
     * Does nothing if Actor does not own any GameplayTags. Does nothing if Actor does not implement IGameplayAssetInterface
     */
    UFUNCTION(BlueprintPure, Category = "AgoraTags")
    static void GetOwnedGameplayTags(AActor* Actor, FGameplayTagContainer& OutContainer);


};
