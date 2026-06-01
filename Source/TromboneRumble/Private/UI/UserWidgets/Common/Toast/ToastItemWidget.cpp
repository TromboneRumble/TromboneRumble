// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/Toast/ToastItemWidget.h"
#include "Subsystems/ToastSubsystem.h"
#include "Subsystems/TweenSubsystem.h"

UToastItemWidget::UToastItemWidget()
	: AnimInDuration(0.15f)
	, AnimOutDuration(0.15f)
	, ToastDisplayDuration(2.0f)
{
}

void UToastItemWidget::InitializeToast(const FToastRequest& Request, const FSimpleDelegate& InToastFinishedCallback)
{
	OnToastFinishedCallback = InToastFinishedCallback;
	ToastDisplayDuration = Request.DisplayDuration;
	
	K2_OnToastInitialized(Request);
	SetRenderScale(FVector2D::ZeroVector);
	
	StartTween(EToastTweenType::AnimIn, AnimInDuration);
}

void UToastItemWidget::DeinitializeToast()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	
	OnToastFinishedCallback.Unbind();
	SetRenderScale(FVector2D::ZeroVector);
	RemoveFromParent();
}

void UToastItemWidget::CloseToastImmediately()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StartCloseTween);
	StartTween(EToastTweenType::AnimOut, 0.0f);
}

void UToastItemWidget::StartTween(const EToastTweenType TweenType, const float InDuration)
{
	FSimpleDelegate TweenCompletionCallback;
	switch (TweenType)
	{
	case EToastTweenType::AnimIn:
		TweenCompletionCallback.BindUObject(this, &ThisClass::HandleAnimInTweenFinished);
		break;
		
	case EToastTweenType::AnimOut:
		TweenCompletionCallback.BindUObject(this, &ThisClass::HandleAnimOutTweenFinished);
		break;
		
	default:
		break;
	}

	const FVector2D TargetScale = (TweenType == EToastTweenType::AnimIn) ? FVector2D::UnitVector : FVector2D::ZeroVector;
	const ETweenCurveType CurveType = (TweenType == EToastTweenType::AnimIn) ? ETweenCurveType::EaseOutBack : ETweenCurveType::Linear;
	
	if (UTweenSubsystem* TweenSubsystem = GetGameInstance()->GetSubsystem<UTweenSubsystem>())
	{
		TweenSubsystem->DoTween(this, TargetScale, InDuration, CurveType, TweenCompletionCallback);
	}
}

void UToastItemWidget::HandleAnimInTweenFinished()
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ThisClass::StartTween, EToastTweenType::AnimOut, AnimOutDuration);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_StartCloseTween, TimerDelegate, ToastDisplayDuration, false);
}

void UToastItemWidget::HandleAnimOutTweenFinished()
{
	const FSimpleDelegate LocalCallback = OnToastFinishedCallback;
	
	DeinitializeToast();
	
	if (UToastSubsystem* ToastSubsystem = GetGameInstance()->GetSubsystem<UToastSubsystem>())
	{
		ToastSubsystem->ReturnToPool(this);
	}
	
	if (LocalCallback.IsBound())
	{
		LocalCallback.Execute();
	}
}

void UToastItemWidget::NativeDestruct()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	
	Super::NativeDestruct();
}
