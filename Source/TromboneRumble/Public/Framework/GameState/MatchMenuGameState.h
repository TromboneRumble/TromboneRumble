#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "UI/UserWidgets/MatchMenu/MatchMenuWidget.h"
#include "MatchMenuGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchTypeChangedSignature, EMatchType, NewType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FK2_OnPlayerCountChangedSignature, int32, NewPlayerCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FK2_OnPlayerStartOccupancyChanged, APlayerStart*, PlayerStart, APawn*, OccupyingPawn);

UCLASS()
class TROMBONERUMBLE_API AMatchMenuGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	AMatchMenuGameState();
	
	void SetMatchType(EMatchType NewType);
	
	/** @return The current player count in the match menu. */
	int32 GetCurrentPlayerCount() const;
	
public:
	/** @return The delegate fired when the player count changed in the match menu */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Player Count Changed", meta = (AllowPrivateAccess))
	FK2_OnPlayerCountChangedSignature OnPlayerCountChangedEvent;
	
	/** @return The delegate fired when a player start occupancy changed in the match menu */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Player Start Occupancy Changed", meta = (AllowPrivateAccess))
	FK2_OnPlayerStartOccupancyChanged OnPlayerStartOccupancyChangedEvent;
	
	FOnMatchTypeChangedSignature OnMatchTypeChanged;
	
public:
	
	/** Tag of the player start where the local player should be moved to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Match")
	FName LocalPlayerStartTag;
	
protected:

	/** PlayerStart where the player can be positioned */
	UPROPERTY()
	TMap<APlayerStart*, APawn*> PlayerStartMappings;
	
public:
	/** Called when a lobby pawn is spawned. */
	virtual void HandleMatchPawnCreated(APawn* PlayerPawn);
	
	/** Called before a lobby pawn is destroyed. */
	virtual void HandleMatchPawnPreDestroyed(APawn* PlayerPawn);

	/** Find a new player start for the given pawn. This is used when we want to have the local player in a fix spot. */
	UFUNCTION(BlueprintCallable, Category = "Default")
	virtual APlayerStart* FindPlayerStart(APawn* PlayerPawn) const;
	
private:
	
	UFUNCTION()
	void OnRep_CurrentMatchType();
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMatchType)
	EMatchType CurrentMatchType = EMatchType::Custom;
	
public:

	//~ Begin AGameStateBase Interface
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AGameStateBase Interface

public:
	
	// ~ Begin Getter & Setter
	FORCEINLINE EMatchType GetCurrentMatchType() const { return CurrentMatchType; }
	// ~ End Getter & Setter
};