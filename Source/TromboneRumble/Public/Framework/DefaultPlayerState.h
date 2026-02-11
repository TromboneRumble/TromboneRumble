// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Utilities/Defines.h"
#include "DefaultPlayerState.generated.h"

class AWeaponBase;
class URhythmSubsystem;
class AInGameState;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLocalScoreChanged, APlayerState*, PlayerState, int32, AddedAmount, EScoreType, ScoreType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboChanged, ENoteResult, InNoteResult, int32, ComboCount);

UCLASS()
class TROMBONERUMBLE_API ADefaultPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ADefaultPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerName() override;
	virtual void OnRep_Score() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;	


	// 클라이언트 PlayerState 에서 점수가 바뀌면 GameState에 알림.
	UPROPERTY(BlueprintAssignable)
	FOnLocalScoreChanged OnLocalScoreChanged;

	UPROPERTY(BlueprintAssignable)
	FOnComboChanged OnComboChanged;

	void AddScore(int32 Amount, EScoreType ScoreType);
	UFUNCTION(Server, Reliable)
	void Server_AddScore(int32 Amount, EScoreType ScoreType);

	void HandleCombo(ENoteResult InResult);

	UPROPERTY(VisibleInstanceOnly, Replicated)
	TSubclassOf<AWeaponBase> EquippedWeaponClass;
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_SkinColor)
	FLinearColor SkinColor = FLinearColor::Black;

	UFUNCTION()
	void OnRep_SkinColor();

	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 CurrentCombo = 0;

public:
	//getter setter
	FORCEINLINE float GetRhythmScore() const { return GetScore(); }
	void SetSkinColor(const FLinearColor& InSkinColor);
	FORCEINLINE FLinearColor GetSkinColor() const { return SkinColor; }
	FORCEINLINE int32 GetCurrentCombo() const { return CurrentCombo; }
	// ~getter setter
};
