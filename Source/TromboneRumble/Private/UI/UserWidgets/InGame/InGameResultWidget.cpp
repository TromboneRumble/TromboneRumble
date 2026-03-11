// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/InGame/InGameResultWidget.h"
#include "Components/TextBlock.h"
#include "Framework/DefaultPlayerState.h"
#include "CommonButtonBase.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "TromboneGamePlayTags.h"
#include "EasySessionSubsystem.h"

void UInGameResultWidget::SetResultData(ADefaultPlayerState* PlayerState, int32 PlayerRank)
{
	if (!PlayerState) return;

	FRumbleScoreData ScoreData = PlayerState->GetScoreData();

	FString RankSuffix;
	switch (PlayerRank)
	{
	case 1: RankSuffix = TEXT("st"); break;
	case 2: RankSuffix = TEXT("nd"); break;
	case 3: RankSuffix = TEXT("rd"); break;
	default: RankSuffix = TEXT("th"); break;
	}
	if (RankText)
	{
		RankText->SetText(FText::FromString(FString::Printf(TEXT("<%d%s>"), PlayerRank, *RankSuffix)));
	}

	// 최종 합산 점수
	if (TotalScoreText)
	{
		TotalScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d Points"), FMath::RoundToInt(PlayerState->GetScore()))));
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

void UInGameResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReturnToMainMenuButton)
	{
		ReturnToMainMenuButton->OnClicked().RemoveAll(this);
		ReturnToMainMenuButton->OnClicked().AddUObject(this, &ThisClass::HandleExitButtonClicked);
	}

	if (!SessionsSubsystem)
	{
		const UGameInstance* GameInstance = GetGameInstance();
		SessionsSubsystem = GameInstance->GetSubsystem<UEasySessionSubsystem>();
		SessionsSubsystem->OnDestroySessionSuccess.AddUObject(this, &ThisClass::OnDestroySessionSuccess);
		SessionsSubsystem->OnDestroySessionFailure.AddUObject(this, &ThisClass::OnDestroySessionFailure);
	}
}

void UInGameResultWidget::HandleExitButtonClicked()
{
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
