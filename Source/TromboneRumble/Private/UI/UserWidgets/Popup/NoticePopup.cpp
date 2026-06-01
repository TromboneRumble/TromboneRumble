#include "UI/UserWidgets/Popup/NoticePopup.h"
#include "CommonTextBlock.h"
#include "UI/UserWidgets/Common/CommonButtonBaseWithText.h"

void UNoticePopup::Init(const FText& InContent, const FText& InTitle, const FText& InCloseButtonText) const
{	
	if (Text_Title)
	{
		Text_Title->SetText(InTitle);
	}
	if (Text_Content)
	{
		Text_Content->SetText(InContent);
	}
	if (Button_Close)
	{
		Button_Close->SetText(InCloseButtonText);
	}
}