// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Data/RhythmSongDataRow.h"
#include "Framework/TromboneGameInstance.h"
#include "Subsystems/GameDataSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmLeaderBoard.h"
#include "UI/UserWidgets/Rhythm/RhythmFloatingScoreWidget.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Framework/DefaultPlayerState.h"
#include "Characters/DefaultPlayerController.h"
#include "Utilities/DebugHelper.h"


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
	if (ComboText)
	{
		ComboText->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ComboNumberText)
	{
		ComboNumberText->SetVisibility(ESlateVisibility::Collapsed);
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
			BindDelegates(DefaultPlayerState);
		}
		else if (ADefaultPlayerController* DefaultPlayerController = Cast<ADefaultPlayerController>(GetOwningPlayer()))
		{
			DefaultPlayerController->OnPlayerStateChanged.AddDynamic(this, &ThisClass::OnPlayerStateChanged);
		}
	}
}

void URhythmUIRootWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}



void URhythmUIRootWidget::BindDelegates(ADefaultPlayerState* InDefaultPlayerState)
{
	InDefaultPlayerState->OnLocalScoreChanged.RemoveDynamic(this, &ThisClass::HandleOnLocalScoreChanged);
	InDefaultPlayerState->OnLocalScoreChanged.AddDynamic(this, &ThisClass::HandleOnLocalScoreChanged);
	InDefaultPlayerState->OnComboChanged.RemoveDynamic(this, &ThisClass::UpdateComboText);
	InDefaultPlayerState->OnComboChanged.AddDynamic(this, &ThisClass::UpdateComboText);
}

void URhythmUIRootWidget::UpdateComboText(ENoteResult InNoteResult, int32 ComboCount)
{
	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::None) return;
	if (InNoteResult == ENoteResult::Bad)
	{
		if (ComboText)
		{
			ComboText->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (ComboNumberText)
		{
			ComboNumberText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		if (ComboText)
		{
			ComboText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (ComboNumberText)
		{
			ComboNumberText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			FString FormattedScore = FString::Printf(TEXT("%d"), ComboCount);
			ComboNumberText->SetText(FText::FromString(FormattedScore));
			if (ComboHitAnim)
			{
				PlayAnimation(ComboHitAnim);
			}
		}
	}
}

void URhythmUIRootWidget::OnPlayerStateChanged(APlayerState* NewPlayerState)
{
	if (ADefaultPlayerState* PS = Cast<ADefaultPlayerState>(NewPlayerState))
	{
		BindDelegates(PS);

		// 더 이상 들을 필요 없으니 구독 해제 (선택사항)
		if (ADefaultPlayerController* DefaultPlayerController = Cast<ADefaultPlayerController>(GetOwningPlayer()))
		{
			DefaultPlayerController->OnPlayerStateChanged.RemoveDynamic(this, &ThisClass::OnPlayerStateChanged);
		}
	}
}


void URhythmUIRootWidget::HandleOnLocalScoreChanged(APlayerState* PlayerState, int32 AddedAmount, EScoreType ScoreType)
{
	UpdateScoreText(PlayerState, AddedAmount, ScoreType);
	SpawnFloatingScoreWidget(PlayerState, AddedAmount, ScoreType);
}

void URhythmUIRootWidget::UpdateScoreText(APlayerState* AffectedPlayerState, int32 AddedAmount, EScoreType ScoreType)
{
	if (!AffectedPlayerState) return;

	if (AController* PC = AffectedPlayerState->GetOwningController())
	{
		if (!PC->IsLocalController()) return;
	}

	if (ScoreText)
	{
		int32 UpdatedScore = FMath::FloorToInt(AffectedPlayerState->GetScore());
		FString FormattedScore = FString::Printf(TEXT("%06d"), UpdatedScore);
		ScoreText->SetText(FText::FromString(FormattedScore));

		if (ScoreType != EScoreType::None && ScoreUpdatedAnim)
		{
			PlayAnimation(ScoreUpdatedAnim);
		}
	}
}

void URhythmUIRootWidget::SpawnFloatingScoreWidget(APlayerState* PlayerState, int32 AddedAmount, EScoreType ScoreType)
{
	if (AddedAmount <= 0) return;

	if (!PlayerState) return;
	AController* PC = PlayerState->GetOwningController();
	if (!PC || !PC->IsLocalController()) return;

	if (ScoreType == EScoreType::None || ScoreType == EScoreType::Invalid) return;

	if (FloatingScoreWidgetClass)
	{
		URhythmFloatingScoreWidget* FloatingWidget = CreateWidget<URhythmFloatingScoreWidget>(this, FloatingScoreWidgetClass);
		if (FloatingWidget)
		{
			FloatingWidget->Init(AddedAmount, ScoreType);

			if (AddedScoreContainer)
			{
				UCanvasPanelSlot* CanvasSlot = AddedScoreContainer->AddChildToCanvas(FloatingWidget);
				if (CanvasSlot)
				{
					CanvasSlot->SetAutoSize(true);

					float RandomX = FMath::RandRange(-20.0f, 20.0f);
					float RandomY = FMath::RandRange(-20.0f, 20.0f);

					CanvasSlot->SetPosition(FVector2D(RandomX, RandomY));
				}
			}
			else
			{
				FloatingWidget->AddToViewport();
			}
		}
	}
}