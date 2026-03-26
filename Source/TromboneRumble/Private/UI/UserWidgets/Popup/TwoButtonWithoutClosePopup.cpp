#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"
#include "CommonTextBlock.h"
#include "UI/UserWidgets/Common/CommonButtonBaseWithText.h"

void UTwoButtonWithoutClosePopup::OnInit(const FText& InTitle, const FText& InContent, const FText& LeftText, const FText& RightText, const FOnPopupAction& InLeftButtonDelegate, const FOnPopupAction& InRightButtonDelegate, const bool bShouldClosePopup)
{
	Init();
	
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
		Button_Left->SetText(LeftText);
	}
	if (Button_Right)
	{
		Button_Right->SetText(RightText);
	}
	
	OnLeftButtonClicked = InLeftButtonDelegate;
	OnRightButtonClicked = InRightButtonDelegate;
	bShouldClosePopupAfterClick = bShouldClosePopup;
}

void UTwoButtonWithoutClosePopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Button_Left)
	{
		Button_Left->OnClicked().AddUObject(this, &ThisClass::HandleLeftButtonClicked);
	}
	if (Button_Right)
	{
		Button_Right->OnClicked().AddUObject(this, &ThisClass::HandleRightButtonClicked);
	}
}

void UTwoButtonWithoutClosePopup::HandleLeftButtonClicked()
{
	if (OnLeftButtonClicked.IsBound())
	{
		OnLeftButtonClicked.Broadcast();
	}
	
	if (bShouldClosePopupAfterClick)
	{
		ClosePopup();
	}
}

void UTwoButtonWithoutClosePopup::HandleRightButtonClicked()
{
	if (OnRightButtonClicked.IsBound())
	{
		OnRightButtonClicked.Broadcast();
	}
	
	if (bShouldClosePopupAfterClick)
	{
		ClosePopup();
	}
}