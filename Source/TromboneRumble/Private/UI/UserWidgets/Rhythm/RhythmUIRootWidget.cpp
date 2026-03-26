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
#include "Framework/InGameState.h"
#include "Utilities/DebugHelper.h"



void URhythmUIRootWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime()) return;
}

void URhythmUIRootWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;
	BindDelegates();
	if (ADefaultPlayerController* DefaultPlayerController = Cast<ADefaultPlayerController>(GetOwningPlayer()))
	{
		DefaultPlayerController->OnPlayerStateChanged.AddDynamic(this, &ThisClass::HandleOnPlayerStateChanged);
	}
}

void URhythmUIRootWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_RetryBind);
		TimerHandle_RetryBind.Invalidate();
	}
	Super::NativeDestruct();
}

void URhythmUIRootWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}



void URhythmUIRootWidget::BindDelegates()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController) return;


	if (ADefaultPlayerState* DefaultPlayerState = PlayerController->GetPlayerState<ADefaultPlayerState>())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_RetryBind);

		DefaultPlayerState->OnLocalScoreChanged.RemoveDynamic(this, &ThisClass::HandleOnLocalScoreChanged);
		DefaultPlayerState->OnLocalScoreChanged.AddDynamic(this, &ThisClass::HandleOnLocalScoreChanged);

		// Debug::Print(TEXT("Successfully Bound to PlayerState Delegates"), FColor::Green);
	}
	else
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_RetryBind))
		{
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle_RetryBind,
				this,
				&URhythmUIRootWidget::RetryBindDelegates,
				0.1f,
				true 
			);
		}
	}
}

void URhythmUIRootWidget::RetryBindDelegates()
{
	// Debug::Print(TEXT("Retrying Delegate Binding..."), FColor::Yellow);
	BindDelegates();
}

void URhythmUIRootWidget::HandleOnPlayerStateChanged(APlayerState* NewPlayerState)
{
	if (ADefaultPlayerState* PS = Cast<ADefaultPlayerState>(NewPlayerState))
	{
		BindDelegates();

		// 더 이상 들을 필요 없으니 구독 해제 (선택사항)
		if (ADefaultPlayerController* DefaultPlayerController = Cast<ADefaultPlayerController>(GetOwningPlayer()))
		{
			DefaultPlayerController->OnPlayerStateChanged.RemoveDynamic(this, &ThisClass::HandleOnPlayerStateChanged);
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

					float RandomX = FMath::RandRange(-40.0f, 60.0f);
					float RandomY = FMath::RandRange(-30.0f, 30.0f);

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