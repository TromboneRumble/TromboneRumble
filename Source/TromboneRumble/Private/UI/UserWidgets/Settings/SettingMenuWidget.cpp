#include "UI/UserWidgets/Settings/SettingMenuWidget.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "UI/UserWidgets/Settings/VideoOptionPanel.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "UI/UserWidgets/Settings/LanguageOptionPanel.h"

UWidget* USettingMenuWidget::NativeGetDesiredFocusTarget() const
{
	if (CB_Audio)
	{
		return CB_Audio;
	}
	return Super::NativeGetDesiredFocusTarget();
}

void USettingMenuWidget::Init()
{
	Super::Init();
	
	if (CB_Audio)
	{
		CB_Audio->OnClicked().RemoveAll(this);
		CB_Audio->OnClicked().AddLambda([this] { ChangePanel(Widget_AudioOptions); });
	}
	if (CB_Video)
	{
		CB_Video->OnClicked().RemoveAll(this);
		CB_Video->OnClicked().AddLambda([this] { ChangePanel(Widget_VideoOptions); });
	}
	if (CB_Language)
	{
		CB_Language->OnClicked().RemoveAll(this);
		CB_Language->OnClicked().AddLambda([this] { ChangePanel(Widget_LanguageOptions); });
	}
	if (CB_Back)
	{
		CB_Back->OnClicked().RemoveAll(this);
		CB_Back->OnClicked().AddUObject(this, &USettingMenuWidget::DeactivateWidget);
		CB_Back->OnClicked().AddLambda([this]()
		{
			GetRootLayout()->PopPopup();
			if (UOptionPanelBase* ActivePanel = Cast<UOptionPanelBase>(CAS_Settings->GetActiveWidget()))
			{
				ActivePanel->Deactivate();
			}
			DeactivateWidget();
		});
	}
}

void USettingMenuWidget::ChangePanel(UWidget* TargetWidget)
{
	if (CAS_Settings)
	{
		CAS_Settings->SetActiveWidget(TargetWidget);
	}	
}