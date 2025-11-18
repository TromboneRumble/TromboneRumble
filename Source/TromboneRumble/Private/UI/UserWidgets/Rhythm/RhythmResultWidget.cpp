// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmResultWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Utilities/Defines.h"
#include "Components/TextBlock.h"
#include "Subsystems/WidgetPoolSubsystem.h"

void URhythmResultWidget::PlayAnimationOnResult(ENoteResult InResult)
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
		case ENoteResult::Great:
			ScoreText->SetColorAndOpacity(FLinearColor{ 0.f,1.f,0.f,1.f });
			break;
		case ENoteResult::Excellent:
			ScoreText->SetColorAndOpacity(FLinearColor{ 0.f,0.f,1.f,1.f });
			break;
		}
		PlayAnimation(Detected);
	}
}

void URhythmResultWidget::NativePreConstruct()
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

void URhythmResultWidget::NativeDestruct()
{
	if (Detected)
	{
		UnbindAllFromAnimationFinished(Detected);
	}
	Super::NativeDestruct();
}

void URhythmResultWidget::OnDetectedAnimationFinished()
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();

	if (UWidgetPoolSubsystem* WidgetPoolSubsystem = LocalPlayer->GetSubsystem<UWidgetPoolSubsystem>())
	{
		WidgetPoolSubsystem->Release(this);
		
	}
}
