// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Popup/PopupWidgetBase.h"
#include "CommonButtonBase.h"

void UPopupWidgetBase::Init()
{
	if (!IsInViewport())
	{
		AddToViewport();
	}
	ActivateWidget();
}

void UPopupWidgetBase::Refresh()
{
}

void UPopupWidgetBase::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	if (Button_Close)
	{
		Button_Close->OnClicked().AddUObject(this, &UPopupWidgetBase::HandleCloseButtonClicked);
	}
	
	if (bCloseDim && Button_Dim)
	{
		Button_Dim->OnClicked().AddUObject(this, &UPopupWidgetBase::HandleCloseButtonClicked);
	}

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
	OnAfterCloseAction.Broadcast();
	
	Super::NativeOnDeactivated();
}

void UPopupWidgetBase::HandleCloseButtonClicked()
{
	ClosePopup();
}

void UPopupWidgetBase::OnCloseAnimationFinished()
{
	UnbindAllFromAnimationFinished(FadeIn);
	DeactivateWidget();
}

void UPopupWidgetBase::ClosePopup(const bool bCloseImmediately)
{
	if (bIsClosing) return;
	
	OnBeforeCloseAction.Broadcast();
	SetEnableButtons(false);
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (bCloseImmediately || !FadeIn)
	{
		DeactivateWidget();
	}
	else
	{
		FWidgetAnimationDynamicEvent EndDelegate;
		EndDelegate.BindDynamic(this, &UPopupWidgetBase::OnCloseAnimationFinished);
		BindToAnimationFinished(FadeIn, EndDelegate);
		PlayAnimationReverse(FadeIn);
	}
}

void UPopupWidgetBase::SetEnableButtons(bool bInIsEnabled)
{
	if (Button_Close) Button_Close->SetIsEnabled(bInIsEnabled);
	if (Button_Dim)   Button_Dim->SetIsEnabled(bInIsEnabled);
}
