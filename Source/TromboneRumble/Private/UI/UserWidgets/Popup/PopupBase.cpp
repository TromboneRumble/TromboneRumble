#include "UI/UserWidgets/Popup/PopupBase.h"
#include "UI/UserWidgets/Common/CommonButtonBaseWithText.h"
#include "CommonButtonBase.h"

UPopupBase::UPopupBase()
{
	bIsBackHandler = true;
}

void UPopupBase::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	bIsClosing = false;
	Register();
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

void UPopupBase::NativeOnDeactivated()
{
	bIsClosing = false;
	Unregister();
	
	Super::NativeOnDeactivated();
}

void UPopupBase::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
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

bool UPopupBase::NativeOnHandleBackAction()
{
	if (bAllowBackAction)
	{
		ClosePopup(false);
	}
	return true;
}

void UPopupBase::Refresh()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UPopupBase::ClosePopup(const bool bCloseImmediately)
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

void UPopupBase::Register()
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

void UPopupBase::Unregister()
{
	if (Button_Close)
	{
		Button_Close->OnClicked().RemoveAll(this);
	}
	
	if (Button_Dim)
	{
		Button_Dim->OnClicked().RemoveAll(this);
	}
}
