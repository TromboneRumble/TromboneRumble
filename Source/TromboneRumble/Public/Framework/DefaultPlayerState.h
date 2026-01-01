// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Utilities/Defines.h"
#include "DefaultPlayerState.generated.h"

class AWeaponBase;
class URhythmSubsystem;
class AInGameState;

USTRUCT(BlueprintType)
struct FComboData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentCombo = 0;

	UPROPERTY(BlueprintReadOnly)
	ENoteResult LastNoteResult = ENoteResult::None; 

	//콤보가 0일때 Bad 판정 칠시 판단 용도로 패킷 구분
	UPROPERTY()
	uint8 TransactionID = 0;

	bool operator==(const FComboData& Other) const
	{
		return CurrentCombo == Other.CurrentCombo &&
			LastNoteResult == Other.LastNoteResult &&
			TransactionID == Other.TransactionID;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalScoreChanged, APlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboChanged, ENoteResult, InNoteResult, int32, ComboCount);

UCLASS()
class TROMBONERUMBLE_API ADefaultPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ADefaultPlayerState();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerName() override;
	virtual void OnRep_Score() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;	


	// 클라이언트 PlayerState 에서 점수가 바뀌면 GameState에 알림.
	UPROPERTY(BlueprintAssignable)
	FOnLocalScoreChanged OnLocalScoreChanged;

	UPROPERTY(BlueprintAssignable)
	FOnComboChanged OnComboChanged;

	void AddScore(int32 Amount);
	UFUNCTION(Server, Reliable)
	void Server_AddScore(int32 Amount);
	UFUNCTION()
	void HandleNoteDetected(ENoteResult InNoteResult);

	UPROPERTY(VisibleInstanceOnly, Replicated)
	TSubclassOf<AWeaponBase> EquippedWeaponClass;
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_SkinColor)
	FLinearColor SkinColor = FLinearColor::Black;

	UFUNCTION()
	void OnRep_SkinColor();

	UPROPERTY(ReplicatedUsing = OnRep_ComboData)
	FComboData ComboData;

	UFUNCTION()
	void OnRep_ComboData();

	void HandleCombo(ENoteResult InResult);

	UFUNCTION(Server, Reliable)
	void Server_HandleCombo(ENoteResult InResult);

public:
	//getter setter
	FORCEINLINE float GetRhythmScore() const { return GetScore(); }
	void SetSkinColor(const FLinearColor& InSkinColor);
	FORCEINLINE FLinearColor GetSkinColor() const { return SkinColor; }
	// ~getter setter
};
