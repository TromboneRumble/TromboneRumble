#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleRowWidget.h"
#include "CommonRotator.h"
#include "CommonTextBlock.h"

void UOptionCycleRowWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (CT_OptionName) CT_OptionName->SetText(OptionRowTitleText);
}

void UOptionCycleRowWidget::ForceInit(const FText InName, const TArray<FText> InOptions, const int32 DefaultIndex)
{
	OptionRowTitleText = InName;
	TextOptions = InOptions;
	DefaultSelectedIndex = DefaultIndex;
	
	if (CT_OptionName) CT_OptionName->SetText(OptionRowTitleText);
	if (CR_Rotator)
	{
		CR_Rotator->PopulateTextLabels(TextOptions);
		CR_Rotator->SetSelectedItem(DefaultSelectedIndex);
	}
}