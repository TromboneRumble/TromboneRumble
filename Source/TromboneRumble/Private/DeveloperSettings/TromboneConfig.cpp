#include "DeveloperSettings/TromboneConfig.h"
#include "UI/UserWidgets/Popup/NoticePopupWidget.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"

UTromboneConfig::UTromboneConfig()
{
	NoticePopupWidgetClass = UNoticePopupWidget::StaticClass();
	TwoButtonWithoutClosePopupWidgetClass = UTwoButtonWithoutClosePopup::StaticClass();
	CharacterSkinColors = { FLinearColor::Red, FLinearColor::Green, FLinearColor::Blue, FLinearColor::Yellow, FLinearColor::White };
}

const UTromboneConfig* UTromboneConfig::Get()
{
	return GetDefault<UTromboneConfig>();
}