#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AgoraTypes.h" // FDamageContext
#include "TeamInterface.h"
#include "AgoraPlayerController.generated.h"

class UAgoraCheatManager;
class AAgoraCharacterBase;
class UAgoraUserWidget;
class UAgoraWaitForPlayersWidget;
class ACameraActor;

// TODO, this but not this
DECLARE_LOG_CATEGORY_EXTERN(AgoraPlayerController, Log, All);

UCLASS()
class AGORA_API AAgoraPlayerController : public APlayerController, public ITeamInterface
{
	GENERATED_BODY()
	
public:
	
    FOnTeamChangedSignature OnTeamChanged;

    AAgoraPlayerController();

    // APlayerController
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* NewPawn) override;
    virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;
    virtual void OnUnPossess() override;
    // ~APlayerController

    // ITeamInterface
    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    ETeam GetTeam() const override;
    UFUNCTION(BlueprintCallable, Category = "TeamInterface")
    void SetTeam(ETeam NewTeam) override;
    virtual FOnTeamChangedSignature& GetTeamChangedDelegate() override;
    // ~ITeamInterface

    /**
     * Called on the server to notify a player that they took damage. Generally passes the information to the owning
     * client's version of their PlayerController.
     * NetData [Server, Client]
     */
    UFUNCTION()
    virtual void NotifyTakeHit(float CurrentHealth, float MaxHealth, const FDamageContext& HitDamage);
    
    /**
     * Called on the server to notify a player that they caused damage. Generally passes the information to the owning
     * client's version of their PlayerController.
     * NetData [Server, Client]
     */
    UFUNCTION()
    virtual void NotifyCausedHit(const FDamageContext& HitDamage);

    UFUNCTION()
    void NotifyPawnRespawned();
    UFUNCTION()
    void NotifyDied(const FDamageContext& KillingBlow);


    UFUNCTION()
    void NotifyMatchFinished(APawn* DestroyedCore);
    /**[Server, Client] */
    UFUNCTION()
    void NotifyHeroKilled(AActor* Hero, const FDamageContext KillingBlow);
    /**[Server, Client] */
    UFUNCTION()
    void NotifyTowerDestroyed(AActor* Tower, const FDamageContext KillingBlow);
    /**[Server, Client] */
    UFUNCTION()
    void NotifyCoreDestroyed(AActor* Core, const FDamageContext KillingBlow);

	// This could be called GetAgoraCharacter, however it'll show up in
	// suggestions when people look for GetPawn this way
	UFUNCTION(BlueprintCallable)
	AAgoraCharacterBase* GetAgoraPawn() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<UAgoraUserWidget> HUDWidgetClass;

    UPROPERTY(BlueprintReadWrite, Category = "HUD")
    UAgoraUserWidget* HUDWidget = nullptr;

    UAgoraCheatManager* AgoraCheatManager = nullptr;

protected:

    // APlayerController
    virtual void BeginPlay() override;
    // ~APlayerController

	UFUNCTION(BlueprintCallable, Category = "Agora|PlayerController")
	virtual void SetSpectating(bool bShouldSpectate = true);

    /**
     * Sets up for a new pawn, sanity checking.
     */
    UFUNCTION()
    virtual void SetupPawn(APawn* NewPawn);

    UFUNCTION()
    void SetupPostDeathPan(AActor* Killer);

    UFUNCTION()
    void UnsetupPostDeathPan();

    UFUNCTION()
    void SetupPostDeathSpectate();

    UFUNCTION()
    void SetupHUD();

    /**
     * Removes this controller's association with a pawn.
     * Generally done when this controller unpossesses/stops viewing another pawn
     */
    UFUNCTION()
    virtual void UnsetupPawn(APawn* PawnToUnset);

    UFUNCTION(Server, Reliable, WithValidation)
    void NotifyServerClientLoaded();

    /**
     * Used so that clients can react to the controlled pawn changing
     */
    UFUNCTION(Client, Reliable, Category = "PlayerController")
    virtual void Client_OnPossess(APawn* NewPawn);

    /**
     * Used so that the clients can react to the controlled pawn changing
     */
    UFUNCTION(Client, Reliable, Category = "PlayerController")
    virtual void Client_UnPossess(APawn* OlPawn);
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerController")
    TArray<TSubclassOf<UUserWidget>> WidgetsToRemoveOnBegin;

    /**
     * Are we currently panning to our killer after dying?
     */
    bool bPostDeathPan = false;

    /**
     * The target we are currently panning towards after dying.
     */
    UPROPERTY()
    TWeakObjectPtr<AActor> PostDeathViewTarget = nullptr;

    /**
     * Singleton-esque object that lets us view our killer after death. Cannot use our hero-pawn as it 
     * A.) might not exist
     * B.) should not be rotating after dying
     */
    ACameraActor* PostDeathPanCamera = nullptr;

    /**
     * Timer handle for our post-death panning.
     */
    FTimerHandle PostDeathPanCameraExpireTimerHandle;


    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraPlayerController|Events")
    void ReceiveHeroKilled(AActor* Hero, const FDamageContext& KillingBlow);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraPlayerController|Events")
    void ReceiveTowerDestroyed(AActor* Hero, const FDamageContext& KillingBlow);
    UFUNCTION(BlueprintImplementableEvent, Category = "AgoraPlayerController|Events")
    void ReceiveCoreDestroyed(AActor* Hero, const FDamageContext& KillingBlow);
    UFUNCTION(BlueprintImplementableEvent, Category = "PlayerController")
    void ReceiveMatchFinished(APawn* DestroyedCore);
    UFUNCTION(BlueprintImplementableEvent, Category = "PlayerController")
    void ReceivePawnDied(const FDamageContext& KillingBlow);



};
