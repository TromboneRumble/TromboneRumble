#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "UI/UserWidgets/MatchMenu/MatchMenuWidget.h"
#include "MatchMenuGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerListUpdateSignature, const TArray<FString>&, PlayerNames);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchTypeChangedSignature, EMatchType, NewType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStartOccupancyChanged, APlayerStart*, PlayerStart, APawn*, OccupyingPawn);

UCLASS()
class TROMBONERUMBLE_API AMatchMenuGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	AMatchMenuGameState();

	void UpdatePlayerList();
	
	void SetMatchType(EMatchType NewType);
	
	FOnMatchTypeChangedSignature OnMatchTypeChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnPlayerStartOccupancyChanged OnPlayerStartOccupancyChanged;
	
public:
	
	/** Tag of the player start where the local player should be moved to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lobby")
	FName LocalPlayerStartTag;
	
protected:
	
	/**
 * Possible PlayerStart actors and the lobby pawns that currently occupy them.
 * Used to figure out which PlayerStarts are free to spawn on when relocating lobby pawns.
 * Not replicated because pawns are moved around locally in each clients world.
 */
	UPROPERTY()
	TMap<APlayerStart*, APawn*> PlayerStartMappings;
	
public:
	/** Called when a lobby pawn is spawned. */
	virtual void HandleLobbyPawnCreated(APawn* PlayerPawn);
	
	/** Called before a lobby pawn is destroyed. */
	virtual void HandleLobbyPawnPreDestroyed(APawn* PlayerPawn);

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