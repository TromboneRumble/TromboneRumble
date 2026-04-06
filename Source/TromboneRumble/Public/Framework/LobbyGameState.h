// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

enum class ELobbyState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyStateChangedSignature, ELobbyState, NewState);

UCLASS()
class TROMBONERUMBLE_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	ALobbyGameState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetLobbyState(ELobbyState NewState);

	// Host가 게임 시작 버튼 누를때 선택한 곡에 따라 애셋 로딩
	UFUNCTION(BlueprintCallable)
	void SetSelectedSongTag(const FGameplayTag& InTag);

public:
	FOnLobbyStateChangedSignature OnLobbyStateChanged;
	
private:
	UFUNCTION()
	void OnRep_LobbyState() const;

	UFUNCTION()
	void OnRep_SelectedSongTag();
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
	ELobbyState CurrentLobbyState;
	ELobbyState PreviousLobbyState;

	UPROPERTY(ReplicatedUsing = OnRep_SelectedSongTag)
	FGameplayTag SelectedSongTag;

public:
	// ~ Begin Getter & Setter
	FORCEINLINE ELobbyState GetCurrentLobbyState() const { return CurrentLobbyState; }
	FORCEINLINE ELobbyState GetPreviousLobbyState() const { return PreviousLobbyState; }
	FORCEINLINE bool IsInState(const ELobbyState State) const { return CurrentLobbyState == State; }
	// ~ End Getter & Setter
};
