#include "UI/UserWidgets/Popup/TwoButtonPopup.h"
#include "CommonTextBlock.h"
#include "UI/UserWidgets/Common/CommonButtonBaseExtensionWithText.h"

void UTwoButtonPopup::Init(const FTwoButtonPopupParams& InParams)
{
	if (Text_Title)
	{
		Text_Title->SetText(InParams.Title);
	}
	if (Text_Content)
	{
		Text_Content->SetText(InParams.Content);
	}
	if (Button_Left)
	{
		Button_Left->SetText(InParams.LeftButtonText);
	}
	if (Button_Right)
	{
		Button_Right->SetText(InParams.RightButtonText);
	}
	
	LeftCallback = InParams.LeftCallback;
	RightCallback = InParams.RightCallback;
	
	bShouldCloseOnLeftButtonClick = InParams.bCloseOnLeftButtonClick;
	bShouldCloseOnRightButtonClick = InParams.bCloseOnRightButtonClick;
}

void UTwoButtonPopup::Register()
{
	Super::Register();
	
	if (Button_Left)
	{
		Button_Left->OnClicked().RemoveAll(this);
		Button_Left->OnClicked().AddUObject(this, &ThisClass::HandleLeftButtonClicked);
	}
	if (Button_Right)
	{
		Button_Right->OnClicked().RemoveAll(this);
		Button_Right->OnClicked().AddUObject(this, &ThisClass::HandleRightButtonClicked);
	}
}

void UTwoButtonPopup::Unregister()
{
	Super::Unregister();
	
	if (Button_Left)
	{
		Button_Left->OnClicked().RemoveAll(this);
	}
	if (Button_Right)
	{
		Button_Right->OnClicked().RemoveAll(this);
	}
}

void UTwoButtonPopup::HandleLeftButtonClicked()
{
	const TFunction<void()> LocalCallback = LeftCallback;
	
	if (bShouldCloseOnLeftButtonClick)
	{
		ClosePopup(true);
	}
	
	if (LocalCallback)
	{
		LocalCallback();
	}
}

void UTwoButtonPopup::HandleRightButtonClicked()
{
	const TFunction<void()> LocalCallback = RightCallback;

	if (bShouldCloseOnRightButtonClick)
	{
		ClosePopup(true);
	}
    
	if (LocalCallback)
	{
		LocalCallback();
	}
}