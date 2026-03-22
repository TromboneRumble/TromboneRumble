// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/InGame/InGameResultWidget.h"
#include "Components/TextBlock.h"
#include "Framework/DefaultPlayerState.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "TromboneGamePlayTags.h"
#include "EasySessionSubsystem.h"
#include "Actors/ResultScene/ResultCutsceneDirector.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/Image.h"

void UInGameResultWidget::SetResultData(ADefaultPlayerState* PlayerState, int32 PlayerRank)
{
	if (!PlayerState) return;

	FRumbleScoreData ScoreData = PlayerState->GetScoreData();

	if (RankImage && RankTextures.IsValidIndex(PlayerRank - 1))
	{
		UTexture2D* TargetTexture = RankTextures[PlayerRank - 1].LoadSynchronous();
		if (TargetTexture)
		{
			RankImage->SetBrushFromTexture(TargetTexture);
		}
	}

	// 최종 합산 점수
	if (TotalScoreText)
	{
		TotalScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::RoundToInt(PlayerState->GetScore()))));
	}

	// 총 연주 점수 파트
	if (RhythmScoreText) RhythmScoreText->SetText(FText::AsNumber(FMath::RoundToInt(ScoreData.TotalScore)));
	if (PerfectCountText) PerfectCountText->SetText(FText::AsNumber(ScoreData.PerfectCount));
	if (GoodCountText) GoodCountText->SetText(FText::AsNumber(ScoreData.GoodCount));
	if (MissCountText) MissCountText->SetText(FText::AsNumber(ScoreData.MissCount));
	if (TromboneBuffScoreText) TromboneBuffScoreText->SetText(FText::AsNumber(FMath::RoundToInt(ScoreData.TromboneComboBuffScore)));
	if (ViolinBuffScoreText) ViolinBuffScoreText->SetText(FText::AsNumber(FMath::RoundToInt(ScoreData.ViolinBuffScore)));

	// 공격 점수 파트
	if (AttackScoreText) AttackScoreText->SetText(FText::AsNumber(FMath::RoundToInt(ScoreData.AttackScore)));
	if (HitCountText) HitCountText->SetText(FText::AsNumber(ScoreData.HitCount));
	if (CymbalsBuffScoreText) CymbalsBuffScoreText->SetText(FText::AsNumber(FMath::RoundToInt(ScoreData.CymbalsAttackScore)));

	// 기타 점수 파트
	if (OtherScoreText) OtherScoreText->SetText(FText::AsNumber(FMath::RoundToInt(ScoreData.OtherScore)));
	if (StealCountText) StealCountText->SetText(FText::AsNumber(ScoreData.InstrumentStealCount));
	if (SpotlightCountText) SpotlightCountText->SetText(FText::AsNumber(ScoreData.SpotlightPickupCount));
}

void UInGameResultWidget::HideSkipButtonAndShowButtons()
{
	if (SkipButton)
	{
		SkipButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ResultOverlay)
	{
		ResultOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		ResultOverlay->SetRenderOpacity(1.f);
		if (ViewMyResultButton)
		{
			ViewMyResultButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			ViewMyResultButton->SetRenderOpacity(1.f);
		}
		if (ReturnToMainMenuButtonLeaderBoard)
		{
			ReturnToMainMenuButtonLeaderBoard->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			ReturnToMainMenuButtonLeaderBoard->SetRenderOpacity(1.f);
		}

		if (ViewLeaderboardButton)
		{
			ViewLeaderboardButton->SetVisibility(ESlateVisibility::Collapsed);
			ViewLeaderboardButton->SetRenderOpacity(0.f);
		}
		if (ReturnToMainMenuButtonMyResult)
		{
			ReturnToMainMenuButtonMyResult->SetVisibility(ESlateVisibility::Collapsed);
			ReturnToMainMenuButtonMyResult->SetRenderOpacity(0.f);
		}
	}
	
}


void UInGameResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;


	if (BackgroundBlurOverlay) BackgroundBlurOverlay->SetRenderOpacity(0.f);
	if (ResultOverlay) ResultOverlay->SetRenderOpacity(0.f);

	if (SkipButton) SkipButton->OnClicked.AddDynamic(this, &ThisClass::HandleSkipClicked);
	if (ViewMyResultButton) ViewMyResultButton->OnClicked.AddDynamic(this, &ThisClass::HandleViewMyResultClicked);
	if (ViewLeaderboardButton) ViewLeaderboardButton->OnClicked.AddDynamic(this, &ThisClass::HandleViewLeaderboardClicked);
	if (ReturnToMainMenuButtonLeaderBoard) ReturnToMainMenuButtonLeaderBoard->OnClicked.AddDynamic(this, &ThisClass::HandleExitButtonClicked);
	if (ReturnToMainMenuButtonMyResult) ReturnToMainMenuButtonMyResult->OnClicked.AddDynamic(this, &ThisClass::HandleExitButtonClicked);

	if (ViewLeaderboardButton)
	{
		ViewLeaderboardButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ReturnToMainMenuButtonMyResult)
	{
		ReturnToMainMenuButtonMyResult->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ViewMyResultButton)
	{
		ViewMyResultButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ReturnToMainMenuButtonLeaderBoard)
	{
		ReturnToMainMenuButtonLeaderBoard->SetVisibility(ESlateVisibility::Collapsed);
	}


	if (!SessionsSubsystem)
	{
		const UGameInstance* GameInstance = GetGameInstance();
		SessionsSubsystem = GameInstance->GetSubsystem<UEasySessionSubsystem>();
		SessionsSubsystem->OnDestroySessionSuccess.AddUObject(this, &ThisClass::OnDestroySessionSuccess);
		SessionsSubsystem->OnDestroySessionFailure.AddUObject(this, &ThisClass::OnDestroySessionFailure);
	}
}

void UInGameResultWidget::HandleSkipClicked()
{
	if (Director.IsValid()) Director->SkipResultSequence();
}

void UInGameResultWidget::HandleViewMyResultClicked()
{

	if (ViewLeaderboardButton)
	{
		ViewLeaderboardButton->SetVisibility(ESlateVisibility::Visible);
	}
	if (ReturnToMainMenuButtonMyResult)
	{
		ReturnToMainMenuButtonMyResult->SetVisibility(ESlateVisibility::Visible);
	}

	if (ViewMyResultButton)
	{
		ViewMyResultButton->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (ReturnToMainMenuButtonLeaderBoard)
	{
		ReturnToMainMenuButtonLeaderBoard->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (Director.IsValid())
	{
		Director->PlayZoomSequence(true); // 줌인
		PlayAnimation(SpawnAnimation);
	}
}

void UInGameResultWidget::HandleViewLeaderboardClicked()
{
	
	if (ViewLeaderboardButton)
	{
		ViewLeaderboardButton->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (ReturnToMainMenuButtonMyResult)
	{
		ReturnToMainMenuButtonMyResult->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (ViewMyResultButton)
	{
		ViewMyResultButton->SetVisibility(ESlateVisibility::Visible);
	}
	if (ReturnToMainMenuButtonLeaderBoard)
	{
		ReturnToMainMenuButtonLeaderBoard->SetVisibility(ESlateVisibility::Visible);
	}

	if (Director.IsValid())
	{
		Director->PlayZoomSequence(false); // 줌아웃 (역재생)
		PlayAnimation(SpawnAnimation, 0, 1, EUMGSequencePlayMode::Reverse);
	}
}

void UInGameResultWidget::HandleExitButtonClicked()
{
	if (Director.IsValid())
	{
		Director->StopBGM();
	}
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (UEasySessionSubsystem* SessionSubsystem = GI->GetSubsystem<UEasySessionSubsystem>())
		{
			SessionSubsystem->DestroySession();
		}
		else if (APlayerController* PC = GetOwningPlayer())
		{
			const FString MainMenuMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMenu_Main);
			PC->ClientTravel(MainMenuMapPath, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UInGameResultWidget::OnDestroySessionSuccess()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		const FString MainMenuMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMenu_Main);
		PC->ClientTravel(MainMenuMapPath, ETravelType::TRAVEL_Absolute);
	}
}

void UInGameResultWidget::OnDestroySessionFailure()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		const FString MainMenuMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMenu_Main);
		PC->ClientTravel(MainMenuMapPath, ETravelType::TRAVEL_Absolute);
	}
}

void UInGameResultWidget::SetDirector(AResultCutsceneDirector* InDirector)
{
	Director = InDirector;
}
