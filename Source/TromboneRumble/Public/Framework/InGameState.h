// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Utilities/Defines.h"
#include "InGameState.generated.h"


enum class EInGameState : uint8;
class ADefaultPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, APlayerState*, UpdatedPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLeaderChanged, APlayerState*, NewLeader, APlayerState*, OldLeader);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateAdded, APlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateRemoved, APlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInGameStateChanged, EInGameState, NewState);

UCLASS()
class TROMBONERUMBLE_API AInGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadCastInGameStateChanged(EInGameState InGameState);

	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnScoreChanged OnScoreChanged;

	// 1등이 바뀌었을때 호출됨
	UPROPERTY(BlueprintAssignable)
	FOnLeaderChanged OnLeaderChanged;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateAdded OnPlayerStateAdded;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateRemoved OnPlayerStateRemoved;

	UPROPERTY(BlueprintAssignable)
	FOnInGameStateChanged OnInGameStateChanged;

	// 클라이언트의 PlayerState점수가 변경되었을 경우, 서버에서 실행해주는 delegate
	UFUNCTION()
	void HandleLocalScoreChanged(APlayerState* UpdatedPlayerState, int32 AddedAmount, EScoreType ScoreType);

	UFUNCTION()
	void HandleScoreChanged(APlayerState* UpdatePlayerState);
	// ~Delegates

	
protected:

	void RecalculateLeader();

	UFUNCTION()
	void OnRep_CurrentLeader(APlayerState* OldLeader);
	
	UFUNCTION()
	void OnRep_CurrentGameState();

	// 현재 1등 플레이어
	UPROPERTY(ReplicatedUsing = OnRep_CurrentLeader)
	APlayerState* CurrentLeader = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentGameState)
	EInGameState CurrentGameState = EInGameState::Initializing;

private:
	
	// 현재 1등을 반환. 없으면 nullptr
	APlayerState* GetTopScoringPlayer() const;
	
public:
	
	// ~ Begin AGameStateBase Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ~ End AGameStateBase Interface

protected:
	
	// ~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~ End AActor Interface
	
public:
	
	void GetPlayersSortedByScore(TArray<APlayerState*>& OutPlayers) const;

	int32 GetPlayerRank(APlayerState* Player) const;

	FORCEINLINE APlayerState* GetCurrentLeader() const { return CurrentLeader; }

	FORCEINLINE EInGameState GetCurrentGameState() const { return CurrentGameState; }
	
};
