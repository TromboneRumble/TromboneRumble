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
	if (ProgressBar)
	{
		ProgressBar->SetRenderOpacity(0.f);
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

void UViolinComboWidget::HandleComboChanged_Implementation(ENoteResult InNoteResult, int32 ComboCount)
{
}

void UViolinComboWidget::HandleBuffStatusChanged_Implementation(bool IsActive)
{
	isBuffActivated = IsActive;
	if (isBuffActivated)
	{
		StopAnimation(ComboBarAnim);
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
