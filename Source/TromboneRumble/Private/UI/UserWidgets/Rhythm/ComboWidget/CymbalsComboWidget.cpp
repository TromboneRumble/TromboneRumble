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
	if (ComboText)
	{
		ComboText->SetRenderOpacity(0.f);
	}
	if (IdleAnim)
	{
		PlayAnimation(IdleAnim,0.f,0);
	}
}

void UCymbalsComboWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UCymbalsComboWidget::HandleComboChanged(ENoteResult InNoteResult, int32 ComboCount)
{
	if (!ComboText) return;
	ComboText->SetRenderOpacity(1.0f);

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
		if (MissAnim)
		{
			StopAllAnimations();
			UUMGSequencePlayer* Player = PlayAnimation(MissAnim);
			Player->OnSequenceFinishedPlaying().AddLambda([this](UUMGSequencePlayer& InSequencePlayer)
				{
					if (IdleAnim)
					{
						PlayAnimation(IdleAnim, 0.0f, 0);
					}
				});
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
	}
	break;

	case ENoteResult::None:
	case ENoteResult::Invalid:
	default:
		break;
	}
}

void UCymbalsComboWidget::HandleOnAttack(AActor* HitActor)
{
	if (OnAttackAnim)
	{
		PlayAnimation(OnAttackAnim);
	}
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
		MyPS->OnComboChanged.RemoveDynamic(this, &ThisClass::HandleComboChanged);
		MyPS->OnComboChanged.AddDynamic(this, &ThisClass::HandleComboChanged);
		OwnerInstrument->OnHitSuccess.RemoveDynamic(this, &ThisClass::HandleOnAttack);
		OwnerInstrument->OnHitSuccess.AddDynamic(this, &ThisClass::HandleOnAttack);
	}
	else
	{
		FTimerHandle WaitHandle;
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, this, &ThisClass::BindDelegates, 0.1f, false);
	}
}
