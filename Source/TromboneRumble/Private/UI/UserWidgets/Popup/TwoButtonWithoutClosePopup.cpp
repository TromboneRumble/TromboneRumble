#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"
#include "CommonTextBlock.h"
#include "UI/UserWidgets/Common/CommonButtonBaseWithText.h"

UTwoButtonWithoutClosePopup::UTwoButtonWithoutClosePopup() 
	: bShouldClosePopupAfterClick(true)
{
}

void UTwoButtonWithoutClosePopup::OnInit(const FText& InTitle, const FText& InContent, const FText& InLeftText, const FText& InRightText, const TFunction<void()> InLeftCallback, const TFunction<void()> InRightCallback, const bool bShouldClosePopup)
{
	if (Text_Title)
	{
		Text_Title->SetText(InTitle);
	}
	if (Text_Content)
	{
		Text_Content->SetText(InContent);
	}
	if (Button_Left)
	{
		Button_Left->SetText(InLeftText);
	}
	if (Button_Right)
	{
		Button_Right->SetText(InRightText);
	}
	
	LeftCallback = InLeftCallback;
	RightCallback = InRightCallback;
	bShouldClosePopupAfterClick = bShouldClosePopup;
}

void UTwoButtonWithoutClosePopup::Register()
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

void UTwoButtonWithoutClosePopup::HandleLeftButtonClicked()
{
	if (bShouldClosePopupAfterClick)
	{
		ClosePopup(true);
	}
	
	if (LeftCallback)
	{
		LeftCallback();
	}
}

void UTwoButtonWithoutClosePopup::HandleRightButtonClicked()
{
	if (bShouldClosePopupAfterClick)
	{
		ClosePopup(true);
	}
	
	if (RightCallback)
	{
		RightCallback();
	}
}