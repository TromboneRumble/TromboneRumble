#include "UI/UserWidgets/Settings/LanguageOptionPanel.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleRowWidget.h"

ULanguageOptionPanel::ULanguageOptionPanel()
	: SupportedCultures( {TEXT("ko"), TEXT("en")}), 
	CurrentLanguageIndex(0)
{
}

void ULanguageOptionPanel::RefreshUI()
{
	Super::RefreshUI();
	
	const FString CurrentCulture = UKismetInternationalizationLibrary::GetCurrentLanguage();
	for (int32 i = 0; i < SupportedCultures.Num(); ++i)
	{
		if (SupportedCultures[i].Equals(CurrentCulture))
		{
			CurrentLanguageIndex = i;
			break;
		}
	}
	
	if (OC_Language)
	{
		OC_Language->SetSelectedIndex(CurrentLanguageIndex);
	}
}

void ULanguageOptionPanel::ReapplySavedSettings()
{
	Super::ReapplySavedSettings();
	
	const FString CurrentCulture = UKismetInternationalizationLibrary::GetCurrentLanguage();
	UKismetInternationalizationLibrary::SetCurrentLanguage(SupportedCultures[CurrentLanguageIndex], true);
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
