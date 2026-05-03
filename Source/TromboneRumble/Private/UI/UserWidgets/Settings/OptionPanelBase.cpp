#include "UI/UserWidgets/Settings/OptionPanelBase.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "CommonButtonBase.h"

void UOptionPanelBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	SaveManagerSubsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	
	Register();
	
	if (bAutoRefreshUIOnActivate)
	{
		RefreshUI();
	}
}

void UOptionPanelBase::NativeDestruct()
{
	Unregister();
	
	if (bAutoReapplySettingsOnDeactivate)
	{
		ReapplySavedSettings();
	}
	
	Super::NativeDestruct();
}

void UOptionPanelBase::Register()
{
	if (Button_Apply)
	{
		Button_Apply->OnClicked().AddUObject(this, &ThisClass::HandleApplyButtonClicked);
	}
	if (Button_Reset)
	{
		Button_Reset->OnClicked().AddUObject(this, &ThisClass::HandleResetButtonClicked);
	}
}

void UOptionPanelBase::Unregister()
{
	if (Button_Apply)
	{
		Button_Apply->OnClicked().RemoveAll(this);
	}
	if (Button_Reset)
	{
		Button_Reset->OnClicked().RemoveAll(this);
	}
}

void UOptionPanelBase::RefreshUI()
{
	// To be overridden by child classes if needed
}

void UOptionPanelBase::ReapplySavedSettings()
{
	// To be overridden by child classes if needed
}

void UOptionPanelBase::HandleApplyButtonClicked()
{
	// To be overridden by child classes if needed
}

void UOptionPanelBase::HandleResetButtonClicked()
{
	// To be overridden by child classes if needed
}
