// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/ComboWidget/TromboneComboWidget.h"

#include "Animation/UMGSequencePlayer.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

#include "Framework/DefaultPlayerState.h"
#include "Items/InstrumentBase.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"



void UTromboneComboWidget::SetPercentSmooth(float NewPercent)
{
	TargetPercent = FMath::Clamp(NewPercent, 0.0f, 1.0f);
}

void UTromboneComboWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;
	BindDelegates();
	if (ProgressBar)
	{
		ProgressBar->SetRenderOpacity(0.f);
		ProgressBar->SetPercent(0.f);
	}
}

void UTromboneComboWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!ProgressBar) return;

	
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

void UTromboneComboWidget::HandleComboChanged_Implementation(ENoteResult InNoteResult, int32 ComboCount)
{
	//if (!ComboText) return;
	//ComboText->SetRenderOpacity(1.0f);
	//switch (InNoteResult)
	//{
	//case ENoteResult::Bad:
	//{
	//	static const TArray<FString> BadPhrases = {
	//		TEXT("Oops!"),
	//		TEXT("Meh"),
	//		TEXT("What?"),
	//		TEXT("No!"),
	//		TEXT("Miss...")
	//	};

	//	int32 RandomIndex = FMath::RandRange(0, BadPhrases.Num() - 1);
	//	ComboText->SetText(FText::FromString(BadPhrases[RandomIndex]));
	//	ComboText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
	//	if (MissAnim)
	//	{
	//		StopAllAnimations();
	//		PlayAnimation(MissAnim);
	//	}
	//}
	//break;

	//case ENoteResult::Good:
	//case ENoteResult::Excellent:
	//{
	//	FString ComboString = FString::FromInt(ComboCount);
	//	ComboText->SetText(FText::FromString(ComboString));
	//	ComboText->SetColorAndOpacity(FLinearColor(FColor::FromHex(TEXT("FF5900FF"))));
	//	if (ComboTextAnim)
	//	{
	//		PlayAnimation(ComboTextAnim);
	//	}
	//	if (!isBuffActivated && ComboBarAnim)
	//	{
	//		ProgressBar->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	//		PlayAnimation(ComboBarAnim);
	//	}
	//}
	//break;

	//case ENoteResult::None:
	//case ENoteResult::Invalid:
	//default:
	//	break;
	//}
}

void UTromboneComboWidget::HandleBuffStatusChanged_Implementation(bool IsActive)
{
	isBuffActivated = IsActive;
	if (isBuffActivated)
	{
		StopAnimation(ComboBarAnim);
		ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		ProgressBar->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UTromboneComboWidget::BindDelegates()
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
