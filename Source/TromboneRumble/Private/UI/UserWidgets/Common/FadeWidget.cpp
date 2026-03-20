#include "UI/UserWidgets/Common/FadeWidget.h"

UFadeWidget::UFadeWidget()
{
	SetIsFocusable(false);
	bSupportsActivationFocus = false;
	UUserWidget::SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UFadeWidget::StartFadeIn()
{
	OnFadeInStarted.Broadcast();
	
	if (FadeAnimation)
	{
		UnbindAllFromAnimationFinished(FadeAnimation);
		
		PlayAnimation(FadeAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward);
		
		FWidgetAnimationDynamicEvent EndEvent;
		EndEvent.BindDynamic(this, &ThisClass::HandleFadeInComplete);
		BindToAnimationFinished(FadeAnimation, EndEvent);
	}
}

void UFadeWidget::StartFadeOut()
{
	OnFadeOutStarted.Broadcast();
	
	if (FadeAnimation)
	{
		UnbindAllFromAnimationFinished(FadeAnimation);

		PlayAnimation(FadeAnimation, 0.0f, 1, EUMGSequencePlayMode::Reverse);
		
		FWidgetAnimationDynamicEvent EndEvent;
		EndEvent.BindDynamic(this, &ThisClass::HandleFadeOutComplete);
		BindToAnimationFinished(FadeAnimation, EndEvent);
	}
}

void UFadeWidget::HandleFadeInComplete()
{
	StartFadeOut();

	OnFadeInComplete.Broadcast();
}

void UFadeWidget::HandleFadeOutComplete()
{
	OnFadeOutComplete.Broadcast();
}

TOptional<FUIInputConfig> UFadeWidget::GetDesiredInputConfig() const
{
	return TOptional<FUIInputConfig>();
}

void UFadeWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	StartFadeIn();
}

void UFadeWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
}