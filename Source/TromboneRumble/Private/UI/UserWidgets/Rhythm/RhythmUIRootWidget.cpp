// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Data/RhythmSongDataRow.h"
#include "Framework/TromboneGameInstance.h"
#include "Subsystems/GameDataSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmLeaderBoard.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Framework/DefaultPlayerState.h"


void URhythmUIRootWidget::OnGameEnded()
{
	if (ShowLeaderboardAnim)
	{
		PlayAnimation(ShowLeaderboardAnim);
	}
	if (WBP_LeaderBoard)
	{
		WBP_LeaderBoard->SetButtonsVisibility(true);
	}
}

void URhythmUIRootWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime()) return;
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnRhythmGameStarted.AddDynamic(this, &ThisClass::OnRhythmGameStarted);
	}
	if (ComboText)
	{
		ComboText->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MusicProgressBar)
	{
		MusicProgressBar->SetPercent(0.f);
	}
}

void URhythmUIRootWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (ADefaultPlayerState* DefaultPlayerState = PlayerController->GetPlayerState<ADefaultPlayerState>())
		{
			DefaultPlayerState->OnLocalScoreChanged.AddDynamic(this, &ThisClass::UpdateScoreText);
		}
	}
}

void URhythmUIRootWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (hasGameStarted)
	{
		UpdateProgressbar(InDeltaTime);
	}
	
}

void URhythmUIRootWidget::UpdateScoreText(APlayerState* AffectedPlayerState)
{
	if (AffectedPlayerState && AffectedPlayerState->GetOwningController() && AffectedPlayerState->GetOwningController()->IsLocalController())
	{
		if (ScoreText && ScoreUpdatedAnim)
		{
			int32 UpdatedScore = FMath::FloorToInt(AffectedPlayerState->GetScore());
			FString FormattedScore = FString::Printf(TEXT("%06d"), UpdatedScore);
			ScoreText->SetText(FText::FromString(FormattedScore));
			PlayAnimation(ScoreUpdatedAnim);
		}
	}
}

void URhythmUIRootWidget::OnRhythmGameStarted()
{
	if (UTromboneGameInstance* TromboneGameInstance = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		if (UGameDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>())
		{
			CurrentSongPlayingID = DataSubsystem->GetCurrentSongPlayingID();
			if (CurrentSongPlayingID)
			{
				CurrentSongTotalLength = DataSubsystem->GetCurrentSongLength();
			}
		}
	}
	if (MusicProgressBar)
	{
		MusicProgressBar->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	CurrentTime = 0.f;
	hasGameStarted = true;
}

void URhythmUIRootWidget::UpdateProgressbar(float DeltaSeconds)
{
	if (CurrentSongTotalLength == 0.f) return;
	CurrentTime += DeltaSeconds;

	float Percent = FMath::Clamp(CurrentTime / CurrentSongTotalLength, 0.f, 1.f);
	if (MusicProgressBar)
	{
		MusicProgressBar->SetPercent(Percent);
	}
}
