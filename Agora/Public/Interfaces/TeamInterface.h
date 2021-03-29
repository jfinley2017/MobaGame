#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "TeamInterface.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamChangedSignature, AActor*, ChangedActor);

UENUM(BlueprintType)
enum class ETeam : uint8
{
	Invalid UMETA(DisplayName = "Invalid"),
	Dusk 	UMETA(DisplayName = "Dusk"),
	Dawn	UMETA(DisplayName = "Dawn"),
	Neutral	UMETA(DisplayName = "Neutral")
};

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UTeamInterface : public UInterface
{
	GENERATED_BODY()
};

class AGORA_API ITeamInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, Category = "TeamInterface")
	virtual ETeam GetTeam() const = 0;

	UFUNCTION(BlueprintCallable, Category = "TeamInterface")
	virtual void SetTeam(ETeam NewTeam) = 0;

    virtual FOnTeamChangedSignature& GetTeamChangedDelegate() = 0;
};
