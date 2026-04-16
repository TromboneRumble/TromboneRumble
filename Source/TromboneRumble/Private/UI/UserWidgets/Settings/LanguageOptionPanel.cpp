#include "UI/UserWidgets/Settings/LanguageOptionPanel.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleWidget.h"

ULanguageOptionPanel::ULanguageOptionPanel()
	: CurrentLanguageIndex(0)
{
	SupportedCultures = { TEXT("ko"), TEXT("en") };
}

void ULanguageOptionPanel::RefreshUI()
{
	Super::RefreshUI();
	
	if (OC_Language)
	{
		OC_Language->SetSelectedIndex(CurrentLanguageIndex);
	}
}

void ULanguageOptionPanel::Activate()
{
	Super::Activate();

	const FString CurrentCulture = UKismetInternationalizationLibrary::GetCurrentLanguage();
	for (int32 i = 0; i < SupportedCultures.Num(); ++i)
	{
		if (SupportedCultures[i].Equals(CurrentCulture))
		{
			CurrentLanguageIndex = i;
			break;
		}
	}
	
	RefreshUI();
}

void ULanguageOptionPanel::Deactivate()
{
	Super::Deactivate();
	
	RefreshUI();
}

void ULanguageOptionPanel::HandleApplyButtonClicked()
{
	Super::HandleApplyButtonClicked();

	const int32 SelectedIndex = OC_Language->GetCurrentIndex();
	if (SelectedIndex >= 0 && SelectedIndex < SupportedCultures.Num())
	{
		const FString NewCulture = SupportedCultures[SelectedIndex];
		UKismetInternationalizationLibrary::SetCurrentLanguage(NewCulture, true);
		CurrentLanguageIndex = SelectedIndex;
	}
}

void ULanguageOptionPanel::HandleResetButtonClicked()
{
	Super::HandleResetButtonClicked();
	
	RefreshUI();
}
