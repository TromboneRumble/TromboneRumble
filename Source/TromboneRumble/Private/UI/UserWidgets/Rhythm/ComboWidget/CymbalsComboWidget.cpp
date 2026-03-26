// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/ComboWidget/CymbalsComboWidget.h"
#include "Animation/UMGSequencePlayer.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

#include "Framework/DefaultPlayerState.h"
#include "Items/InstrumentBase.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/Defines.h"

void UCymbalsComboWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;
	BindDelegates();
}

void UCymbalsComboWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(BindRetryTimerHandle);
		BindRetryTimerHandle.Invalidate();
	}
	Super::NativeDestruct();
}

void UCymbalsComboWidget::HandleComboChanged_Implementation(ENoteResult InNoteResult, int32 ComboCount)
{
	
}


void UCymbalsComboWidget::BindDelegates()
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
		GetWorld()->GetTimerManager().ClearTimer(BindRetryTimerHandle);

		MyPS->OnComboChanged.RemoveDynamic(this, &ThisClass::HandleComboChanged);
		MyPS->OnComboChanged.AddDynamic(this, &ThisClass::HandleComboChanged);
		OwnerInstrument->OnHitSuccess.RemoveDynamic(this, &ThisClass::HandleOnAttack);
		OwnerInstrument->OnHitSuccess.AddDynamic(this, &ThisClass::HandleOnAttack);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
			BindRetryTimerHandle,
			this,
			&ThisClass::BindDelegates,
			0.1f,
			false
		);
	}
}
