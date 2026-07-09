// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerReadyCheckComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerReadySignature, APlayerController* /* ReadyPlayer */);
DECLARE_MULTICAST_DELEGATE(FOnAllPlayersReadySignature);

/** UPlayerReadyCheckComponent
 * GameMode에 부착되는 서버 전용 컴포넌트.
 * 각 플레이어의 로딩 완료를 추적해 개별 준비 완료(OnPlayerReady)와 전원 준비 완료(OnAllPlayersReady, 1회)를 알린다.
 */
UCLASS()
class TROMBONERUMBLE_API UPlayerReadyCheckComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Default Constructor */
	UPlayerReadyCheckComponent();

	/** 로딩 완료 추적을 시작. 호스트는 즉시 준비 완료 처리 */
	void StartTracking();

	bool IsPlayerReady(const APlayerController* PC) const { return ReadyPlayers.Contains(PC); }
	int32 GetReadyPlayerCount() const { return ReadyPlayers.Num(); }
	const TArray<TObjectPtr<APlayerController>>& GetReadyPlayers() const { return ReadyPlayers; }

public:

	/** Fired when a player is ready */
	FOnPlayerReadySignature OnPlayerReady;

	/** Fired when all players are ready */
	FOnAllPlayersReadySignature OnAllPlayersReady;

private:

	void HandlePlayerLoadingScreenFinished(APlayerController* PC);

	/** @return Current expected player count */
	int32 GetExpectedPlayerCount() const;

private:

	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> ReadyPlayers;

	/** All ready notification is only once */
	bool bAllReadyBroadcasted = false;

public:

	// ~ Begin UActorComponent Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~ End UActorComponent Interface

};
