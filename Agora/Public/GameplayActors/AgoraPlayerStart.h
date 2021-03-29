#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "TeamInterface.h"
#include "AgoraPlayerStart.generated.h"

/**
 * Just extends team interface so that GM knows which side to spawn players on
 */
UCLASS()
class AGORA_API AAgoraPlayerStart : public APlayerStart, public ITeamInterface
{
	GENERATED_BODY()
	
public:

    // Begin TeamInterface
    FOnTeamChangedSignature TeamChanged;

    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    ETeam GetTeam() const override;
    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    void SetTeam(ETeam NewTeam) override;

    virtual FOnTeamChangedSignature& GetTeamChangedDelegate() override;
    // ~TeamInterface

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	ETeam Team;
};
