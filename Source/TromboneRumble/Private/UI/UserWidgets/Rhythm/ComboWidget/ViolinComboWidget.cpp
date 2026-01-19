// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/ComboWidget/ViolinComboWidget.h"
#include "Animation/UMGSequencePlayer.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

#include "Framework/DefaultPlayerState.h"
#include "Items/InstrumentBase.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/Defines.h"

void UViolinComboWidget::SetPercentSmooth(float NewPercent)
{
	TargetPercent = FMath::Clamp(NewPercent, 0.0f, 1.0f);
}

void UViolinComboWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;
	BindDelegates();
	if (ComboText)
	{
		ComboText->SetRenderOpacity(0.f);
	}
	if (ProgressBar)
	{
		ProgressBar->SetPercent(0.f);
	}
}

void UViolinComboWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (FMath::IsNearlyEqual(CurrentPercent, TargetPercent, 0.001f))
	{
		if (CurrentPercent != TargetPercent)
		{
			CurrentPercent = TargetPercent;
			ProgressBar->SetPercent(CurrentPercent);
		}
		return;
	}

	switch (InterpType)
	{
	case EBarInterpType::Smooth:
		// 목표에 가까워질수록 느려짐
		CurrentPercent = FMath::FInterpTo(CurrentPercent, TargetPercent, InDeltaTime, InterpSpeed);
		break;

	case EBarInterpType::Constant:
		// 일정한 속도로 이동
		CurrentPercent = FMath::FInterpConstantTo(CurrentPercent, TargetPercent, InDeltaTime, InterpSpeed);
		break;
	}

	ProgressBar->SetPercent(CurrentPercent);
}

void UViolinComboWidget::HandleComboChanged(ENoteResult InNoteResult, int32 ComboCount)
{
	if (!ComboText) return;
	ComboText->SetRenderOpacity(1.0f);

	if (IdleAnim && IsAnimationPlaying(IdleAnim))
	{
		StopAnimation(IdleAnim);
	}
	

	switch (InNoteResult)
	{
	case ENoteResult::Bad:
	{
		static const TArray<FString> BadPhrases = {
			TEXT("Oops!"),
			TEXT("Meh"),
			TEXT("What?"),
			TEXT("No!"),
			TEXT("Miss...")
		};

		int32 RandomIndex = FMath::RandRange(0, BadPhrases.Num() - 1);
		ComboText->SetText(FText::FromString(BadPhrases[RandomIndex]));
		ComboText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		if (isBuffActivated && MissTextAnim)
		{
			PlayAnimation(MissTextAnim);
		}
		else if (MissAnim)
		{
			StopAllAnimations();
			PlayAnimation(MissAnim);
		}
	}
	break;

	case ENoteResult::Good:
	{
		FString ComboString = FString::FromInt(ComboCount) + TEXT(" ♪");
		ComboText->SetText(FText::FromString(ComboString));

		ComboText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f))); // Gold
		if (ComboTextAnim)
		{
			PlayAnimation(ComboTextAnim);
		}
		if (!isBuffActivated && ComboBarAnim)
		{
			PlayAnimation(ComboBarAnim);
		}
	}
	break;

	case ENoteResult::Excellent:
	{
		FString ComboString = FString::FromInt(ComboCount) + TEXT(" ♪");
		ComboText->SetText(FText::FromString(ComboString));

		ComboText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f)));
		if (ComboTextAnim)
		{
			PlayAnimation(ComboTextAnim);
		}
		if (!isBuffActivated && ComboBarAnim)
		{
			PlayAnimation(ComboBarAnim);
		}
	}
	break;

	case ENoteResult::None:
	case ENoteResult::Invalid:
	default:
		break;
	}
	UE_LOG(LogTemp, Warning, TEXT("%f"), ProgressBar->GetRenderTransformAngle());
}

void UViolinComboWidget::HandleBuffStatusChanged(bool IsActive)
{
	isBuffActivated = IsActive;
	if (isBuffActivated && BuffActivateAnim)
	{
		StopAnimation(ComboBarAnim);
		UUMGSequencePlayer* Player = PlayAnimation(BuffActivateAnim);
		if (Player)
		{
			Player->OnSequenceFinishedPlaying().AddUObject(this, &ThisClass::OnBuffActivateAnimationFinished);
		}
	}
	else if (!isBuffActivated)
	{
		if (BuffLoopAnim && IsAnimationPlaying(BuffLoopAnim))
		{
			StopAnimation(BuffLoopAnim);
			if (IdleAnim)
			{
				PlayAnimation(IdleAnim);
			}
		}
	}
}

void UViolinComboWidget::OnBuffActivateAnimationFinished(UUMGSequencePlayer& Player)
{
	if (isBuffActivated && BuffLoopAnim)
	{
		PlayAnimation(BuffLoopAnim, 0.0f, 0);
	}
}

void UViolinComboWidget::BindDelegates()
{
	APlayerController* OwningPC = GetOwningPlayer();
	if (!OwningPC) return;

	ADefaultPlayerState* MyPS = OwningPC->GetPlayerState<ADefaultPlayerState>();
	URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>();
	APawn* OwnerPawn = GetOwningPlayerPawn();
	ATromboneCharacterBase* TromboneCharacter = nullptr;
	if (OwnerPawn)
	{
		TromboneCharacter = Cast<ATromboneCharacterBase>(OwnerPawn);
	}

	if (MyPS && RhythmSubsystem && OwnerInstrument.Get() && TromboneCharacter)
	{
		MyPS->OnComboChanged.RemoveDynamic(this, &ThisClass::HandleComboChanged);
		MyPS->OnComboChanged.AddDynamic(this, &ThisClass::HandleComboChanged);
		OwnerInstrument->OnBuffStateChanged.RemoveDynamic(this, &ThisClass::HandleBuffStatusChanged);
		OwnerInstrument->OnBuffStateChanged.AddDynamic(this, &ThisClass::HandleBuffStatusChanged);
	}
	else
	{
		FTimerHandle WaitHandle;
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, this, &ThisClass::BindDelegates, 0.1f, false);
	}
}
