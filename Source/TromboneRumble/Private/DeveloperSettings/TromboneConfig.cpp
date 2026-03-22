#include "DeveloperSettings/TromboneConfig.h"
#include "UI/UserWidgets/Popup/NoticePopupWidget.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"

UTromboneConfig::UTromboneConfig()
{
	NoticePopupWidgetClass = UNoticePopupWidget::StaticClass();
	TwoButtonWithoutClosePopupWidgetClass = UTwoButtonWithoutClosePopup::StaticClass();
}

const UTromboneConfig* UTromboneConfig::Get()
{
	return GetDefault<UTromboneConfig>();
}