#include "UI/UserWidgets/Settings/LanguageOptionPanel.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleWidget.h"

ULanguageOptionPanel::ULanguageOptionPanel()
{
	SupportedCultures = { TEXT("ko"), TEXT("en") };
	CurrentLanguageIndex = 0;
}

void ULanguageOptionPanel::Init()
{
	Super::Init();
	
	FString CurrentCulture = UKismetInternationalizationLibrary::GetCurrentLanguage();
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

void ULanguageOptionPanel::Deactivate()
{
	FString CurrentCulture = UKismetInternationalizationLibrary::GetCurrentLanguage();
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
	
	Super::Deactivate();
}

void ULanguageOptionPanel::HandleApplyButtonClicked()
{
	Super::HandleApplyButtonClicked();
	
	int32 SelectedIndex = OC_Language->GetCurrentIndex();
	if (SelectedIndex >= 0 && SelectedIndex < SupportedCultures.Num())
	{
		FString NewCulture = SupportedCultures[SelectedIndex];
		UKismetInternationalizationLibrary::SetCurrentLanguage(NewCulture, true);
		CurrentLanguageIndex = SelectedIndex;
		UE_LOG(LogTemp, Log, TEXT("Language changed to %s"), *NewCulture);
	}
}

void ULanguageOptionPanel::HandleResetButtonClicked()
{
	Super::HandleResetButtonClicked();
	
	if (OC_Language)
	{
		OC_Language->SetSelectedIndex(CurrentLanguageIndex);
	}
}
