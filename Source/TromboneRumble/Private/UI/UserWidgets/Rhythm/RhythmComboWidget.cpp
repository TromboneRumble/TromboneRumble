// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmComboWidget.h"

#include "Components/TextBlock.h"
#include "Framework/DefaultPlayerState.h"
#include "Utilities/Defines.h"

void URhythmComboWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsDesignTime()) return;
	BindDelegates();
    ComboText->SetRenderOpacity(0.0f);
}

void URhythmComboWidget::HandleComboChanged(ENoteResult InNoteResult, int32 ComboCount)
{
    if (!ComboText) return;

    ComboText->SetRenderOpacity(1.0f);

    switch (InNoteResult)
    {
    case ENoteResult::Bad:
        ComboText->SetText(FText::FromString(TEXT("Oops!")));
        ComboText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        if (MissAnim)
        {
            PlayAnimation(MissAnim);
        }
        break;

    case ENoteResult::Good:
        ComboText->SetText(FText::AsNumber(ComboCount));
        ComboText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f))); // Gold

        if (BounceAnim)
        {
            PlayAnimation(BounceAnim);
        }
        break;

    case ENoteResult::Excellent:
        ComboText->SetText(FText::AsNumber(ComboCount));
        ComboText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f)));
        if (BounceAnim)
        {
            PlayAnimation(BounceAnim);
        }
        break;

    case ENoteResult::None:
    case ENoteResult::Invalid:
    default:
        break;
    }
}

void URhythmComboWidget::BindDelegates()
{
	APlayerController* OwningPC = GetOwningPlayer();
	if (!OwningPC) return;

	ADefaultPlayerState* MyPS = OwningPC->GetPlayerState<ADefaultPlayerState>();

	if (MyPS)
	{
		MyPS->OnComboChanged.RemoveDynamic(this, &ThisClass::HandleComboChanged);
		MyPS->OnComboChanged.AddDynamic(this, &ThisClass::HandleComboChanged);
	}
	else
	{
		FTimerHandle WaitHandle;
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, this, &ThisClass::BindDelegates, 0.1f, false);
	}
}
