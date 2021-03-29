#include "AgoraAILobbyController.h"
#include "AgoraPlayerState.h"

AAgoraAILobbyController::AAgoraAILobbyController()
{
	//ConstructorHelpers::FClassFinder<AAgoraCharacterBase> GadgetFinder(TEXT("/Game/Blueprints/Heroes/Gadget/BP_Gadget"));
	//HeroClass = GadgetFinder.Class;
}

void AAgoraAILobbyController::MakeHeroPick()
{
	GetPlayerState<AAgoraPlayerState>()->SetHeroClass(HeroClass);
}

void AAgoraAILobbyController::OnSelectHero(TSubclassOf<AAgoraCharacterBase> HeroSelected)
{
	AAgoraPlayerState* PS = GetPlayerState<AAgoraPlayerState>();
	check(PS);

	PS->SetHeroClass(HeroSelected);
}
