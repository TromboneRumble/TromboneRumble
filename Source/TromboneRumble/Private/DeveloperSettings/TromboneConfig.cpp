#include "DeveloperSettings/TromboneConfig.h"
#include "Data/UIData.h"
#include "UI/UserWidgets/Popup/NoticePopupWidget.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"

UTromboneConfig::UTromboneConfig()
	: NoticePopupWidgetClass(UNoticePopupWidget::StaticClass()),
	TwoButtonWithoutClosePopupWidgetClass(UTwoButtonWithoutClosePopup::StaticClass()),
	CharacterSkinColors( {FLinearColor::Red}),
	LobbyCountdownTimeSeconds(5), 
	ToastSystemPolicy(EToastSystemPolicy::Queue),
	IntervalAfterQuestCompletion(1.5f), 
	TutorialStartDelay(1.0f),
	PerformanceWidgetUpdateInterval(0.5f)
{
}

const UTromboneConfig* UTromboneConfig::Get()
{
	return GetDefault<UTromboneConfig>();
}