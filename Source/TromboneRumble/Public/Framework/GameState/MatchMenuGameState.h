// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "UI/UserWidgets/MatchMenu/MatchMenuWidget.h"
#include "MatchMenuGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchTypeChangedSignature, EMatchType, NewType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedMapChangedSignature, FGameplayTag, NewMapTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FK2_OnPlayerCountChangedSignature, int32, NewPlayerCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FK2_OnPlayerStartOccupancyChanged, APlayerStart*, PlayerStart, APawn*, OccupyingPawn);

UCLASS()
class TROMBONERUMBLE_API AMatchMenuGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	/** Default constructor. */
	AMatchMenuGameState();
	
	/** Set MatchType(Public or Private). Server Only, Replicated to all clients. */
	void SetMatchType(EMatchType NewType);
	
	/** Set LobbyMapTag. Server Only, Replicated to all clients. */
	void SetSelectedLobbyMap(const FGameplayTag NewLobbyTag);

	/** @return The current player count in the match menu. */
	int32 GetCurrentPlayerCount() const;
	
public:
	
	/** @return The delegate fired when the player count changed in the match menu */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Player Count Changed", meta = (AllowPrivateAccess))
	FK2_OnPlayerCountChangedSignature OnPlayerCountChangedEvent;
	
	/** @return The delegate fired when a player start occupancy changed in the match menu */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Player Start Occupancy Changed", meta = (AllowPrivateAccess))
	FK2_OnPlayerStartOccupancyChanged OnPlayerStartOccupancyChangedEvent;
	
	/** Fired when the MatchType changed (for UI sync on all clients) */
	FOnMatchTypeChangedSignature OnMatchTypeChanged;

	/** Fired when the selected lobby map changed (for UI sync on all clients). */
	FOnSelectedMapChangedSignature OnSelectedMapChanged;

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
	void HandleMatchPawnCreated(APawn* PlayerPawn);
	
	/** Called before a lobby pawn is destroyed. */
	void HandleMatchPawnPreDestroyed(APawn* PlayerPawn);

	/** Find a new player start for the given pawn. This is used when we want to have the local player in a fix spot. */
	UFUNCTION(BlueprintCallable, Category = "Default")
	APlayerStart* FindPlayerStart(APawn* PlayerPawn) const;
	
private:
	
	UFUNCTION()
	void OnRep_CurrentMatchType();
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMatchType)
	EMatchType CurrentMatchType = EMatchType::Custom;

	UFUNCTION()
	void OnRep_SelectedLobbyMap();
	UPROPERTY(ReplicatedUsing = OnRep_SelectedLobbyMap)
	FGameplayTag SelectedLobbyMapTag;

public:

	//~ Begin AGameStateBase Interface
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AGameStateBase Interface

public:
	
	// ~ Begin Getter & Setter
	FORCEINLINE EMatchType GetCurrentMatchType() const { return CurrentMatchType; }
	FORCEINLINE FGameplayTag GetSelectedLobbyMap() const { return SelectedLobbyMapTag; }
	// ~ End Getter & Setter
};