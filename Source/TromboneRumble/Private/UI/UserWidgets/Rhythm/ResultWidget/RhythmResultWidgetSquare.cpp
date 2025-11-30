// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/ResultWidget/RhythmResultWidgetSquare.h"
#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetSquare.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "Utilities/Defines.h"


void URhythmResultWidgetSquare::PlayAnimationOnResult(ENoteResult InResult)
{
	if (ScoreText && Detected)
	{
		FString EnumName = StaticEnum<ENoteResult>()->GetNameStringByValue(static_cast<int64>(InResult));
		ScoreText->SetText(FText::FromString(EnumName));

		switch (InResult)
		{
		case ENoteResult::Bad:
			ScoreText->SetColorAndOpacity(FLinearColor{ 1.f,0.f,0.f,1.f });
			break;
		case ENoteResult::Good:
			ScoreText->SetColorAndOpacity(FLinearColor{ 1.f,1.f,0.f,1.f });
			break;
		case ENoteResult::Excellent:
			ScoreText->SetColorAndOpacity(FLinearColor{ 0.f,0.f,1.f,1.f });
			break;
		}
		PlayAnimation(Detected);
	}
}

void URhythmResultWidgetSquare::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (!IsDesignTime())
	{
		if (Detected)
		{
			FWidgetAnimationDynamicEvent OnAnimationFinished;
			OnAnimationFinished.BindUFunction(this, FName("OnDetectedAnimationFinished"));
			BindToAnimationFinished(Detected, OnAnimationFinished);
		}

	}
}

void URhythmResultWidgetSquare::NativeDestruct()
{
	if (Detected)
	{
		UnbindAllFromAnimationFinished(Detected);
	}
	Super::NativeDestruct();
}

void URhythmResultWidgetSquare::OnDetectedAnimationFinished()
{
	if (OwnerSpawnWidget.IsValid())
	{
		OwnerSpawnWidget->ReleasePooledRhythmResultWidget(this);
	}
	else
	{
		RemoveFromParent();
	}
}
