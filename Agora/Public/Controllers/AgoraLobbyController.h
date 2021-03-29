#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AgoraLobbyController.generated.h"

class AAgoraCharacterBase;

/**
 * Can be used for AI or player
 */
UCLASS()
class AGORA_API AAgoraLobbyController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void OnSelectHero(TSubclassOf<AAgoraCharacterBase> HeroSelected);

	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartGame();

	virtual void BeginPlay() override;
};
