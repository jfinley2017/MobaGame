#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AgoraAILobbyController.generated.h"

class AAgoraPlayerState;
class AAgoraCharacterBase;

/**
 * Just a stub which selects gadget for the AI in lobby currently
 */
UCLASS()
class AGORA_API AAgoraAILobbyController : public AAIController
{
	GENERATED_BODY()
	
public:
	AAgoraAILobbyController();

	UFUNCTION(BlueprintCallable)
	void MakeHeroPick();

	void OnSelectHero(TSubclassOf<AAgoraCharacterBase> HeroSelected);

private:
	TSubclassOf<AAgoraCharacterBase> HeroClass;
};
