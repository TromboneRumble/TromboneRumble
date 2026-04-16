#include "UI/UserWidgets/Settings/OptionPanelBase.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "CommonButtonBase.h"

void UOptionPanelBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	SaveManagerSubsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	
	Register();
	Activate();
}

void UOptionPanelBase::NativeDestruct()
{
	Deactivate();
	Unregister();
	
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

void UOptionPanelBase::Activate()
{
	// To be overridden by child classes if needed
}

void UOptionPanelBase::Deactivate()
{
	// To be overridden by child classes if needed
}

void UOptionPanelBase::HandleApplyButtonClicked()
{
	// To be overridden by child classes
}

void UOptionPanelBase::HandleResetButtonClicked()
{
	// To be overridden by child classes
}
