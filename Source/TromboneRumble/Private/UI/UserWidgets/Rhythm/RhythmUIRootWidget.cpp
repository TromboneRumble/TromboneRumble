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
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnRhythmGameStarted.AddDynamic(this, &ThisClass::OnRhythmGameStarted);
	}
	if (ComboText)
	{
		ComboText->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ComboNumberText)
	{
		ComboNumberText->SetVisibility(ESlateVisibility::Collapsed);
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
	if (hasGameStarted)
	{
		UpdateProgressbar(InDeltaTime);
	}
	
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
		// 1. 점수 텍스트 갱신 (항상 최신 점수로)
		int32 UpdatedScore = FMath::FloorToInt(AffectedPlayerState->GetScore());
		FString FormattedScore = FString::Printf(TEXT("%06d"), UpdatedScore);
		ScoreText->SetText(FText::FromString(FormattedScore));

		// 2. ScoreType에 따른 디버그 메시지 출력 (Debug::Print 사용)
		// InKey를 -1로 설정하여 메시지가 덮어씌워지지 않고 로그처럼 쌓이게 함 (빠른 판정 확인용)
		switch (ScoreType)
		{
		case EScoreType::RhythmScore:
			Debug::Print(FString::Printf(TEXT("[UI] 리듬 판정: +%d"), AddedAmount), -1, FColor::Cyan);
			break;

		case EScoreType::BuffedRhythmScore:
			Debug::Print(FString::Printf(TEXT("[UI] 버프 점수: +%d"), AddedAmount), -1, FColor::Magenta);
			break;

		case EScoreType::InstrumentPickedUp:
			Debug::Print(FString::Printf(TEXT("[UI] 악기 획득: +%d"), AddedAmount), -1, FColor::Green);
			break;

		case EScoreType::OnHit:
			Debug::Print(FString::Printf(TEXT("[UI] 타격(PVP): +%d"), AddedAmount), -1, FColor::Red);
			break;

		case EScoreType::SpotLight:
			// 스포트라이트는 눈에 잘 띄게 노란색으로 설정
			Debug::Print(FString::Printf(TEXT("[UI] 스포트라이트 보너스! +%d"), AddedAmount), -1, FColor::Yellow);
			break;

		case EScoreType::None:
			// Server OnRep에 의한 단순 동기화 시점에는 로그를 남기지 않음
			break;

		default:
			break;
		}

		// 3. 애니메이션 실행 (단순 동기화가 아닐 때만)
		if (ScoreType != EScoreType::None && ScoreUpdatedAnim)
		{
			PlayAnimation(ScoreUpdatedAnim);
		}
	}
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

void URhythmUIRootWidget::BindDelegates(ADefaultPlayerState* InDefaultPlayerState)
{
	InDefaultPlayerState->OnLocalScoreChanged.RemoveDynamic(this, &ThisClass::UpdateScoreText);
	InDefaultPlayerState->OnLocalScoreChanged.AddDynamic(this, &ThisClass::UpdateScoreText);
	InDefaultPlayerState->OnComboChanged.RemoveDynamic(this, &ThisClass::UpdateComboText);
	InDefaultPlayerState->OnComboChanged.AddDynamic(this, &ThisClass::UpdateComboText);
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
