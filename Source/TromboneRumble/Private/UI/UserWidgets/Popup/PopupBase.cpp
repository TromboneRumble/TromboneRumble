#include "UI/UserWidgets/Popup/PopupBase.h"

#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
#include "CommonButtonBase.h"
#include "UI/UserWidgets/Common/CommonButtonBaseExtensionWithText.h"

UPopupBase::UPopupBase()
{
	bIsBackHandler = true;
	bAutoRestoreFocus = true;
}

UWidget* UPopupBase::NativeGetDesiredFocusTarget() const
{
	return GetDefaultFocusWidget();
}

UWidget* UPopupBase::GetDefaultFocusWidget() const
{
	return FirstFocusCandidate({ Button_Close });
}

UWidget* UPopupBase::FirstFocusCandidate(std::initializer_list<UWidget*> Candidates)
{
	for (UWidget* Candidate : Candidates)
	{
		if (Candidate && Candidate->GetIsEnabled() && Candidate->IsVisible())
		{
			return Candidate;
		}
	}
	return nullptr;
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

	if (bPlaySound && OpenSound)
	{
		UAkGameplayStatics::PostEvent(OpenSound, GetOwningPlayerPawn(), 0, FOnAkPostEventCallback());
	}
}

void UPopupBase::NativeOnDeactivated()
{
	if (bPlaySound && CloseSound)
	{
		UAkGameplayStatics::PostEvent(CloseSound, GetOwningPlayerPawn(), 0, FOnAkPostEventCallback());
	}
	
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
