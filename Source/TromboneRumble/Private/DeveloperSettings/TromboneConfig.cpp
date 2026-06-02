#include "DeveloperSettings/TromboneConfig.h"
#include "Data/UIData.h"
#include "UI/UserWidgets/Popup/TwoButtonPopup.h"

UTromboneConfig::UTromboneConfig()
	: NoticePopupWidgetClass(UNoticePopup::StaticClass()),
	TwoButtonPopupWidgetClass(UTwoButtonPopup::StaticClass()),
	CharacterSkinColors( {FLinearColor::Red}),
	RoomCodeLength(5), 
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