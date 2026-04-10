// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmTimeWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Framework/TromboneGameInstance.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/RhythmSubsystem.h"


void URhythmTimeWidget::ResetProgressBar()
{
	CurrentTime = 0.f;
	if (MusicProgressBar)
	{
		MusicProgressBar->SetPercent(0.f);
	}
}

void URhythmTimeWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime()) return;

	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnRhythmGameStateChanged.AddDynamic(this, &ThisClass::HandleRhythmGameStateChanged);
	}

	if (MusicProgressBar)
	{
		MusicProgressBar->SetPercent(0.f);
	}
	if (TimeText)
	{
		TimeText->SetText(FText::FromString(TEXT("0:00 / 0:00")));
	}
}

void URhythmTimeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bHasGameStarted && !bIsSongPaused)
	{
		UpdateTimeUI(InDeltaTime);
	}
}

void URhythmTimeWidget::HandleRhythmGameStateChanged(ERhythmGameState RhythmGameState)
{
	switch (RhythmGameState)
	{
	case ERhythmGameState::Start:
	{
		if (UGameDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>())
		{
			CurrentSongPlayingID = DataSubsystem->GetCurrentSongPlayingID();
			if (CurrentSongPlayingID && CurrentSongPlayingID != AK_INVALID_PLAYING_ID)
			{
				CurrentSongTotalLength = DataSubsystem->GetCurrentSongLength();
			}
		}

		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CurrentTime = 0.f;
		bHasGameStarted = true;
		bIsSongPaused = false;
		break;
	}
	case ERhythmGameState::Paused:
		bIsSongPaused = true;
		break;
	case ERhythmGameState::Resumed:
		bIsSongPaused = false;
		break;
	case ERhythmGameState::Ended:
		bHasGameStarted = false;
		break;
	default:;
	}
}

void URhythmTimeWidget::UpdateTimeUI(float DeltaSeconds)
{
	if (CurrentSongPlayingID == 0 || CurrentSongPlayingID == AK_INVALID_PLAYING_ID) return;
	if (CurrentSongTotalLength <= 0.f) return;
	if (!GetWorld() || GetWorld()->IsPaused()) return;

	CurrentTime += DeltaSeconds;
	CurrentTime = FMath::Clamp(CurrentTime, 0.f, CurrentSongTotalLength);

	// 퍼센트 계산
	float Percent = FMath::Clamp(CurrentTime / CurrentSongTotalLength, 0.f, 1.f);
	UpdateProgressVisuals(Percent);

	// 텍스트 포맷팅 (예: 1:30 / 3:45)
	int32 CurMin = FMath::FloorToInt(CurrentTime / 60.f);
	int32 CurSec = FMath::FloorToInt(CurrentTime) % 60;

	int32 TotMin = FMath::FloorToInt(CurrentSongTotalLength / 60.f);
	int32 TotSec = FMath::FloorToInt(CurrentSongTotalLength) % 60;

	FString FormattedTime = FString::Printf(TEXT("%d:%02d / %d:%02d"), CurMin, CurSec, TotMin, TotSec);

	if (TimeText)
	{
		TimeText->SetText(FText::FromString(FormattedTime));
	}

	//PostAKEvent로 실행한 PlayingID가 Invalid로 뜨는 오류가 있어서
	//하단의 코드는 적용 불가능
	//AkInt32 CurrentPositionMS = 0;
	//AKRESULT eResult = AK::SoundEngine::GetSourcePlayPosition(CurrentSongPlayingID, &CurrentPositionMS);

	//if (eResult == AK_Success)
	//{

	//	float CurrentTimeSeconds = CurrentPositionMS / 1000.f;
	//	float Percent = FMath::Clamp(CurrentTimeSeconds / CurrentSongTotalLength, 0.f, 1.f);
	//	if (MusicProgressBar)
	//	{
	//		MusicProgressBar->SetPercent(Percent);
	//	}

	//	int32 Minutes = FMath::FloorToInt(CurrentTimeSeconds / 60.f);
	//	int32 Seconds = FMath::FloorToInt(CurrentTimeSeconds) % 60;
	//	UE_LOG(LogTemp, Log, TEXT("재생 시간: %02d:%02d"), Minutes, Seconds);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("GetSourcePlayPosition Failed! Result Code: %d"), (int32)eResult);
	//}
}

void URhythmTimeWidget::UpdateProgressVisuals(float Percent)
{
	if (MusicProgressBar)
	{
		MusicProgressBar->SetPercent(Percent);

		// 네모 상자(Thumb)를 Progress Bar 너비에 비례하여 수동으로 이동
		if (ProgressThumb)
		{
			// Progress Bar의 실제 화면상 크기
			FVector2D PBSize = MusicProgressBar->GetCachedGeometry().GetLocalSize();

			// 퍼센트에 맞춰 X축 이동 거리를 계산
			float TargetX = PBSize.X * Percent;
			ProgressThumb->SetRenderTranslation(FVector2D(TargetX, 0.f));
		}
	}
}
