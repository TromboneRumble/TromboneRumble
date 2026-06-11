// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "InGameResultWidget.generated.h"


struct FPlayerResultSceneData;
struct FRumbleScoreData;
class UImage;
class UCanvasPanel;
class UOverlay;
class AResultCutsceneDirector;
class UButton;
class UEasySessionSubsystem;
class UTextBlock;
class ADefaultPlayerState;


/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UInGameResultWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown, EMouseLockMode::LockAlways,false);
	}

	void SetResultData(const FPlayerResultSceneData& InResultData, int32 PlayerRank);

	void HideSkipButtonAndShowButtons();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleSkipClicked();           // 화면 전체 투명 버튼
	UFUNCTION()
	void HandleViewMyResultClicked();   // 내 결과 보기
	UFUNCTION()
	void HandleViewLeaderboardClicked(); // 순위표 보기 (상세에서 뒤로가기)
	UFUNCTION()
	void HandleExitButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RankImage;

	UPROPERTY(EditAnywhere, Category = "UI|Result")
	TArray<TSoftObjectPtr<UTexture2D>> RankTextures;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalScoreText;

	// --- 총 연주 점수 관련 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RhythmScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ExcellentCountText;

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
	TObjectPtr<UButton> SkipButton;            
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> BackgroundBlurOverlay;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> ResultOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ViewMyResultButton;    
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ViewLeaderboardButton; 

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnToMainMenuButtonMyResult;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnToMainMenuButtonLeaderBoard;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> SpawnAnimation;


private:
	
	TWeakObjectPtr<AResultCutsceneDirector> Director;
	
public:
	void SetDirector(AResultCutsceneDirector* InDirector);
};
