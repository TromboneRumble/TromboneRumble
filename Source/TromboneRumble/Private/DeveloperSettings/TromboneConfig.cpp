#include "DeveloperSettings/TromboneConfig.h"

#include "Items/InstrumentCymbals.h"
#include "Items/InstrumentTrombone.h"
#include "Items/InstrumentViolin.h"
#include "UI/UserWidgets/Popup/NoticePopupWidget.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"
#include "Utilities/Defines.h"

UTromboneConfig::UTromboneConfig()
{
	NoticePopupWidgetClass = UNoticePopupWidget::StaticClass();
	TwoButtonWithoutClosePopupWidgetClass = UTwoButtonWithoutClosePopup::StaticClass();
	InstrumentClasses.Add(EWeaponType::Trombone, AInstrumentTrombone::StaticClass());
	InstrumentClasses.Add(EWeaponType::Cymbals, AInstrumentCymbals::StaticClass());
	InstrumentClasses.Add(EWeaponType::Violin, AInstrumentViolin::StaticClass());
}

const UTromboneConfig* UTromboneConfig::Get()
{
	return GetDefault<UTromboneConfig>();
}