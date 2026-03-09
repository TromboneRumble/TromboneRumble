// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameResultWidget.generated.h"


class UEasySessionSubsystem;
class UCommonButtonBase;
class UTextBlock;
class ADefaultPlayerState;


/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UInGameResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Result")
	void SetResultData(ADefaultPlayerState* PlayerState, int32 PlayerRank);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleExitButtonClicked();

	UFUNCTION()
	void OnDestroySessionSuccess();

	UFUNCTION()
	void OnDestroySessionFailure();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RankText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalScoreText;

	// --- 총 연주 점수 관련 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RhythmScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PerfectCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoodCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MissCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TromboneBuffScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ViolinBuffScoreText;

	// --- 공격 점수 관련 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AttackScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HitCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CymbalsBuffScoreText;

	// --- 기타 점수 관련 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> OtherScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StealCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpotlightCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> ReturnToMainMenuButton;

private:
	UPROPERTY(Transient)
	TObjectPtr<UEasySessionSubsystem> SessionsSubsystem;
};
