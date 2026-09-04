#include "UI/UserWidgets/Settings/SettingPopup.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "Framework/TromboneGameInstance.h"
#include "Groups/CommonButtonGroupBase.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "UI/UserWidgets/Settings/OptionPanelBase.h"
#include "UI/UserWidgets/Settings/GameplayOptionPanel.h"
#include "UI/UserWidgets/Settings/VideoOptionPanel.h"
#include "UI/UserWidgets/Settings/LanguageOptionPanel.h"
#include "Utilities/TromboneStatics.h"

UWidget* USettingPopup::GetDefaultFocusWidget() const
{
	// Panels are swapped via SetActiveWidget and never activated, so route focus through the active panel ourselves
	if (CAS_Settings)
	{
		if (const UOptionPanelBase* ActivePanel = Cast<UOptionPanelBase>(CAS_Settings->GetActiveWidget()))
		{
			if (UWidget* PanelFocus = ActivePanel->GetDesiredFocusTarget())
			{
				return PanelFocus;
			}
		}
	}

	if (Button_Apply)
	{
		return Button_Apply;
	}
	return Super::GetDefaultFocusWidget();
}

void USettingPopup::ChangePanel(UWidget* TargetWidget) const
{
	if (CAS_Settings)
	{
		CAS_Settings->SetActiveWidget(TargetWidget);
	}
}

void USettingPopup::HandleActivePanelChanged(UWidget* ActiveWidget, const int32 /*Index*/)
{
	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	if (!InputSubsystem || InputSubsystem->GetCurrentInputType() != ECommonInputType::Gamepad) return;
	
	if (const UOptionPanelBase* Panel = Cast<UOptionPanelBase>(ActiveWidget))
	{
		if (UWidget* FocusWidget = Panel->GetDesiredFocusTarget())
		{
			FocusWidget->SetFocus();
		}
	}
}

void USettingPopup::OnClickApply()
{
	for (const auto& Panel : OptionPanels)
	{
		if (Panel)
		{
			Panel->ApplySettingsFromUI(true);
		}
	}
}

void USettingPopup::OnClickReset()
{
	for (const auto& Panel : OptionPanels)
	{
		if (Panel)
		{
			Panel->ApplySettingsFromSavedData();
			Panel->RefreshUI();
		}
	}
}

void USettingPopup::Register()
{
	Super::Register();
	
	// Tab clicks reach HandleTabSelected through the button group. LB / RB take the same path
	if (!PrevTabActionRow.IsNull())
	{
		PrevTabActionHandle = RegisterUIActionBinding(FBindUIActionArgs(PrevTabActionRow, FSimpleDelegate::CreateWeakLambda(this, [this]
		{
			if (CategoryButtonGroup) CategoryButtonGroup->SelectPreviousButton();
		})));
	}
	if (!NextTabActionRow.IsNull())
	{
		NextTabActionHandle = RegisterUIActionBinding(FBindUIActionArgs(NextTabActionRow, FSimpleDelegate::CreateWeakLambda(this, [this]
		{
			if (CategoryButtonGroup) CategoryButtonGroup->SelectNextButton();
		})));
	}
	
	if (Button_Apply)
	{
		Button_Apply->OnClicked().RemoveAll(this);
		Button_Apply->OnClicked().AddUObject(this, &USettingPopup::OnClickApply);
	}
	
	if (Button_Reset)
	{
		Button_Reset->OnClicked().RemoveAll(this);
		Button_Reset->OnClicked().AddUObject(this, &USettingPopup::OnClickReset);
	}
	
	OptionPanels = { Widget_AudioOptions, Widget_VideoOptions, Widget_LanguageOptions, Widget_GameplayOptions };
}

void USettingPopup::Unregister()
{
	Super::Unregister();
	
	if (PrevTabActionHandle.IsValid())
	{
		PrevTabActionHandle.Unregister();
	}
	if (NextTabActionHandle.IsValid())
	{
		NextTabActionHandle.Unregister();
	}
	if (Button_Apply)
	{
		Button_Apply->OnClicked().RemoveAll(this);
	}
	if (Button_Reset)
	{
		Button_Reset->OnClicked().RemoveAll(this);
	}
}

void USettingPopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (CAS_Settings)
	{
		CAS_Settings->OnActiveWidgetIndexChanged.RemoveAll(this);
		CAS_Settings->OnActiveWidgetIndexChanged.AddUObject(this, &ThisClass::HandleActivePanelChanged);
	}
	
	if (CategoryButtonGroup)
	{
		CategoryButtonGroup->RemoveAll();
	}
	
	CategoryButtonGroup = NewObject<UCommonButtonGroupBase>(this);
	if (CategoryButtonGroup)
	{
		if (CB_Audio) CategoryButtonGroup->AddWidget(CB_Audio);
		if (CB_Video) CategoryButtonGroup->AddWidget(CB_Video);
		if (CB_Language) CategoryButtonGroup->AddWidget(CB_Language);
		if (CB_Gameplay) CategoryButtonGroup->AddWidget(CB_Gameplay);
		
		CategoryButtonGroup->NativeOnSelectedButtonBaseChanged.AddUObject(this, &ThisClass::HandleTabSelected);
		
		if (CB_Audio)
		{
			CB_Audio->SetIsSelected(true, false);
		}
		
		CategoryButtonGroup->SetSelectionRequired(true);
	}
}

void USettingPopup::HandleTabSelected(UCommonButtonBase* Button, const int32 /*Index*/)
{
	if (Button == CB_Audio)         ChangePanel(Widget_AudioOptions);
	else if (Button == CB_Video)    ChangePanel(Widget_VideoOptions);
	else if (Button == CB_Language) ChangePanel(Widget_LanguageOptions);
	else if (Button == CB_Gameplay) ChangePanel(Widget_GameplayOptions);
}

void USettingPopup::ClosePopup(const bool bCloseImmediately)
{
	if (IsAnyPanelDirty() && GetWorld())
	{
		Super::ClosePopup(true);
		ShowIsDirtyNoticePopup();
		return;
	}
	
	Super::ClosePopup(bCloseImmediately);
}

bool USettingPopup::NativeOnHandleBackAction()
{
	if (IsAnyPanelDirty() && GetWorld())
	{
		Super::ClosePopup(true);
		ShowIsDirtyNoticePopup();
		return true;
	}
	
	return Super::NativeOnHandleBackAction();
}

bool USettingPopup::IsAnyPanelDirty() const
{
	for (const auto& Panel : OptionPanels)
	{
		if (Panel && Panel->IsDirty())
		{
			return true;
		}
	}
	
	return false;
}

void USettingPopup::ShowIsDirtyNoticePopup()
{
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{		
		if (UTwoButtonPopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
		{
			FTwoButtonPopupParams Params;
			Params.Content = GI->GetCommonUIText(TEXT("SettingPopup_AskConfirmation"));
			Params.LeftButtonText = GI->GetCommonUIText(TEXT("Common_Yes"));
			Params.RightButtonText = GI->GetCommonUIText(TEXT("Common_No"));
		
			Params.LeftCallback = [this]()
			{
				OnClickApply();
			};
		
			Params.RightCallback = [this]()
			{
				OnClickReset();
			};
			
			Popup->Init(Params);
		}
	}
}
