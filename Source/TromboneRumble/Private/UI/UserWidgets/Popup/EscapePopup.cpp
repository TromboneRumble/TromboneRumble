#include "UI/UserWidgets/Popup/EscapePopup.h"
#include "EasyOnlineSession.h"
#include "UI/UserWidgets/Common/CommonButtonBaseWithText.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/TromboneStatics.h"

void UEscapePopup::Register()
{
	Super::Register();
	
	if (Button_Option)
	{
		Button_Option->OnClicked().RemoveAll(this);
		Button_Option->OnClicked().AddUObject(this, &ThisClass::HandleOptionButtonClicked);
	}
	if (Button_Disconnect)
	{
		Button_Disconnect->OnClicked().RemoveAll(this);
		Button_Disconnect->OnClicked().AddUObject(this, &ThisClass::HandleDisconnectButtonClicked);
	}
}

void UEscapePopup::Unregister()
{
	Super::Unregister();
	
	if (Button_Option)
	{
		Button_Option->OnClicked().RemoveAll(this);
	}
	if (Button_Disconnect)
	{
		Button_Disconnect->OnClicked().RemoveAll(this);
	}
}

void UEscapePopup::HandleOptionButtonClicked() const
{
	const USettingPopup* Popup = UTromboneStatics::ShowPopup<USettingPopup>(GetWorld());
	if (!Popup)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to show setting popup"));
		return;
	}
}

void UEscapePopup::HandleDisconnectButtonClicked() const
{
	if (UTwoButtonPopup* ConfirmPopup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
	{
		FTwoButtonPopupParams Params;
		Params.Title = ConfirmTitle;
		Params.Content = ConfirmDescription;
		Params.LeftButtonText = ConfirmLeftButton;
		Params.RightButtonText = ConfirmRightButton;
	
		Params.RightCallback = [this]()
		{
			if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
			{
				OnlineSession->LeaveGameSession();
			}
		};
	
		ConfirmPopup->Init(Params);
	}
}
