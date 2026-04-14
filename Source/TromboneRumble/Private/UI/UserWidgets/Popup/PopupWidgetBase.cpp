#include "UI/UserWidgets/Popup/PopupWidgetBase.h"
#include "CommonButtonBase.h"

UPopupWidgetBase::UPopupWidgetBase()
	: bCloseDim(true),
	bPlaySound(true),
	bPlayAnimation(true),
	bIsClosing(false)
{
}

void UPopupWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Register();
}


void UPopupWidgetBase::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	bIsClosing = false;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	
	if (bPlayAnimation && FadeIn)
	{
		PlayAnimation(FadeIn);
	}

	if (bPlaySound)
	{
		// TODO: PlaySound
	}
		
}

void UPopupWidgetBase::NativeOnDeactivated()
{
	bIsClosing = false;
	
	Super::NativeOnDeactivated();
}

void UPopupWidgetBase::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);
	
	if (Animation == FadeIn)
	{
		if (IsAnimationPlayingForward(FadeIn))
		{
			OnPopupOpenedEvent.Broadcast();
		}
		else
		{
			DeactivateWidget();
		}
	}
}

void UPopupWidgetBase::Refresh()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UPopupWidgetBase::ClosePopup(const bool bCloseImmediately)
{
	if (bIsClosing) return;
	
	bIsClosing = true;
	OnPopupClosedEvent.Broadcast();
	
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (bCloseImmediately || !FadeIn)
	{
		DeactivateWidget();
	}
	else
	{
		PlayAnimationReverse(FadeIn);
	}
}

void UPopupWidgetBase::Register()
{
	if (Button_Close)
	{
		Button_Close->OnClicked().RemoveAll(this);
		Button_Close->OnClicked().AddUObject(this, &ThisClass::ClosePopup, false);
	}
	
	if (Button_Dim)
	{
		Button_Dim->SetIsEnabled(bCloseDim);
		if (bCloseDim)
		{
			Button_Dim->OnClicked().RemoveAll(this);
			Button_Dim->OnClicked().AddUObject(this, &ThisClass::ClosePopup, false);
		}
	}
}