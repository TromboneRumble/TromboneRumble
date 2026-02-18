// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmFloatingScoreWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void URhythmFloatingScoreWidget::Init(int32 ScoreAmount, EScoreType ScoreType)
{
	if(ScoreVisuals.Contains(ScoreType))
	{
		FScoreVisualData VisualData = ScoreVisuals[ScoreType];

		if (ScoreIcon && VisualData.IconTexture)
		{
			ScoreIcon->SetBrushFromTexture(VisualData.IconTexture);
		}

		if (ScoreText)
		{
			ScoreText->SetColorAndOpacity(FSlateColor(VisualData.TextColor));
		}
	}
	else
	{
		if (ScoreIcon) ScoreIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ScoreText)
	{
		FString FormattedText = FString::Printf(TEXT("+%d"), ScoreAmount);
		ScoreText->SetText(FText::FromString(FormattedText));
	}
}

void URhythmFloatingScoreWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;
	if (FloatAndFadeAnim)
	{
		PlayAnimation(FloatAndFadeAnim);
	}
}

void URhythmFloatingScoreWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);
	if (Animation == FloatAndFadeAnim)
	{
		RemoveFromParent();
	}
}
