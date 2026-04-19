#include "UI/UserWidgets/Popup/EscapePopup.h"
#include "EasyOnlineSession.h"
#include "Framework/TromboneGameInstance.h"
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

void UEscapePopup::HandleOptionButtonClicked()
{
	const USettingPopup* Popup = UTromboneStatics::ShowPopup<USettingPopup>(GetWorld());
	if (!Popup)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to show setting popup"));
		return;
	}
}

void UEscapePopup::HandleDisconnectButtonClicked()
{
	UTwoButtonWithoutClosePopup* ConfirmPopup = UTromboneStatics::ShowPopup<UTwoButtonWithoutClosePopup>(GetWorld());
	if (!ConfirmPopup)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to show disconnect confirmation popup"));
		return;
	}
	
	if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		const FText Title = ConfirmTitle;
		const FText Description = ConfirmDescription;
		const FText LeftButtonText = ConfirmLeftButton;
		const FText RightButtonText = ConfirmRightButton;

		const TFunction<void()> LeftCallback = [this, ConfirmPopup]()
		{
			ConfirmPopup->ClosePopup();
		};
		const TFunction<void()> RightCallback = [this]()
		{
			if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
			{
				OnlineSession->LeaveGameSession();
			}
		};
		
		ConfirmPopup->OnInit(Title, Description, LeftButtonText, RightButtonText, LeftCallback, RightCallback);
	}
	
	LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to leave game session"));
}
