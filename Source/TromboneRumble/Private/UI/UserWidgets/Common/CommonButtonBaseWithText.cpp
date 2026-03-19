#include "UI/UserWidgets/Common/CommonButtonBaseWithText.h"
#include "CommonTextBlock.h"

void UCommonButtonBaseWithText::SetText(const FText& InText) const
{
	if (CT_ButtonText)
	{
		CT_ButtonText->SetText(InText);
	}
}
