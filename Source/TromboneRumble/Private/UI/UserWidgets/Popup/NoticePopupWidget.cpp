#include "UI/UserWidgets/Popup/NoticePopupWidget.h"
#include "CommonTextBlock.h"

UNoticePopupWidget::UNoticePopupWidget()
	: DefaultTitle(TEXT(""))
{
}

void UNoticePopupWidget::OnInit(const FText& InContent) const
{
	if (Text_Title)
	{
		Text_Title->SetText(FText::FromString(DefaultTitle));
	}
	if (Text_Content)
	{
		Text_Content->SetText(InContent);
	}
}

void UNoticePopupWidget::OnInit(const FText& InTitle, const FText& InContent) const
{
	if (Text_Title)
	{
		Text_Title->SetText(InTitle);
	}
	if (Text_Content)
	{
		Text_Content->SetText(InContent);
	}
}
