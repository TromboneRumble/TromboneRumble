// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utilities/Defines.h"
#include "LobbyDirectorComponent.generated.h"

class AGameModeBase;
class AItemBase;
class ALobbyGameState;

DECLARE_MULTICAST_DELEGATE(FOnLobbyTravelCountdownFinishedSignature);

/** ULobbyDirectorComponent
 *  LobbyGameMode에 부착되는 서버 전용 컴포넌트로, 로비 연출 전체를 담당.
 */
UCLASS()
class TROMBONERUMBLE_API ULobbyDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "LobbyDirector", meta = (DisplayName = "전원 로딩 완료 후 낙하까지 대기 시간"))
	float FallStartDelay = 1.0f;

	UPROPERTY(EditAnywhere, Category = "LobbyDirector", meta = (DisplayName = "낙하 지점 미배치 시 시작 높이 fallback"))
	float FallHeight = 1000.0f;

	/** Maximum angle the character is laid down by when the fall starts (degrees). If 0, the character falls standing up. */
	UPROPERTY(EditAnywhere, Category = "LobbyDirector", meta = (DisplayName = "낙하 기울기 각도", ClampMin = "0.0", ClampMax = "90.0"))
	float FallAngle = 45.0f;

	/** Maximum rotational speed during free fall (rad/s). If 0, falls without rotating. */
	UPROPERTY(EditAnywhere, Category = "LobbyDirector", meta = (DisplayName = "낙하 회전 속도", ClampMin = "0.0"))
	float FallRotationRate = 1.5f;

	UPROPERTY(EditAnywhere, Category = "LobbyDirector", meta = (DisplayName = "최대 접지 대기 시간"))
	float MaxGetUpWaitTime = 10.0f;

public:

	/** Default Constructor */
	ULobbyDirectorComponent();

	/** Start lobby flow */
	void StartLobbyFlow();

	/** Stop all timers and lobby flow */
	void AbortLobbyFlow() const;

	/** Handle single player is ready */
	void HandlePlayerReady(APlayerController* ReadyPlayer);

	/** Handle all players are ready */
	void HandleAllPlayersReady();

	void NotifyItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem);

public:

	/** Fired when the server travel countdown is finished. */
	FOnLobbyTravelCountdownFinishedSignature OnTravelCountdownFinished;

	/** Drops every standing player from the falling points again. Debug only, used by Trombone_RagdollDrop. */
	void RelaunchAllPlayersFalling();

private:

	void SetLobbyState(const ELobbyState& InNewState);

	// ~ Begin WaitingForPlayers
	/** Hide the pawn and lock the input. If the pawn is not valid, retry on the next tick. */
	void PrepareWaitingPlayer(APlayerController* PC, int32 RetryCount);
	
	void OnPreFallTimerFinished();
	// ~ End WaitingForPlayers

	// ~ Begin FallingPlayers
	/** Collect the falling points placed on the map */
	void CollectFallingSpawnPoints();
	
	/** Drop all the pawns at the same time */
	void LaunchAllPlayersFalling();
	
	/** Teleports to the falling point and begins the ragdoll. If the pawn is not valid, retry on the next tick. */
	void LaunchPlayerFalling(APlayerController* PC, int32 RetryCount);
	
	/** Start polling to check if every player has grounded in a ragdoll state */
	void StartGroundedPolling();
	
	/** If all players grounded, transit to CountdownToStandup. */
	void PollAllGrounded();
	// ~ End FallingPlayers

	// ~ Begin CountdownToStandup
	void OnStandupCountdownFinished();
	// ~ End CountdownToStandup

	// ~ Begin InstrumentScramble
	void SpawnInstruments();
	// ~ End InstrumentScramble

	void OnTravelCountdownTimerFinished();

	bool IsInLobbyState(ELobbyState State) const;

private:

	UPROPERTY()
	TObjectPtr<AGameModeBase> OwnerGameMode;

	UPROPERTY()
	TObjectPtr<ALobbyGameState> LobbyGameState;

	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> WaitingPlayers;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> FallingSpawnPoints;

	/** Index of the falling point to be assigned to the next player */
	int32 NextFallingSpawnPointIndex = 0;

	/** Number of Instruments spawned in the lobby. */
	int32 SpawnedInstrumentCount = 0;

	/** Number of Instruments equipped by players in the lobby. */
	int32 EquippedInstrumentCount = 0;

	/** Time elapsed since the grounded polling */
	float GroundPollElapsed = 0.0f;

	FTimerHandle PreFallTimerHandle;
	FTimerHandle GroundPollTimerHandle;
	FTimerHandle StandupTimerHandle;
	FTimerHandle TravelTimerHandle;

	/** if pawn is not valid, retry max count */
	static constexpr int32 MaxPawnRetryCount = 30;

	/** All-players grounded polling interval (seconds) */
	static constexpr float GroundPollInterval = 1.0f;

public:

	// ~ Begin UActorComponent Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~ End UActorComponent Interface

};
