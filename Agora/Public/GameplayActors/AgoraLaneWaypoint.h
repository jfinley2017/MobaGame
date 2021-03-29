#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "AgoraTypes.h"
#include "AgoraLaneWaypoint.generated.h"


UCLASS()
class AGORA_API AAgoraLaneWaypoint : public ATargetPoint
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Defaults, meta = (AllowPrivateAccess = "true"))
	ELaneEnum Lane;
};
