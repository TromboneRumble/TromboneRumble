#include "UI/UserWidgets/Settings/LanguageOptionPanel.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleRowWidget.h"

void ULanguageOptionPanel::Register()
{
	Super::Register();
	
	if (OC_Language)
	{
		OC_Language->OnRotatedWithDirection().RemoveAll(this);
		OC_Language->OnRotatedWithDirection().AddDynamic(this, &ThisClass::OnLanguageRotated);
	}
}

void ULanguageOptionPanel::Unregister()
{
	Super::Unregister();
	
	if (OC_Language)
	{
		OC_Language->OnRotatedWithDirection().RemoveAll(this);
	}
}

void ULanguageOptionPanel::RefreshUI()
{
	Super::RefreshUI();
	
	int32 TargetIndex = 0;
	const FString CurrentCulture = UKismetInternationalizationLibrary::GetCurrentLanguage();
	for (int32 i = 0; i < SupportedCultures.Num(); ++i)
	{
		if (SupportedCultures[i].Equals(CurrentCulture))
		{
			TargetIndex = i;
			break;
		}
	}
	
	if (OC_Language)
	{
		OC_Language->SetSelectedIndex(TargetIndex);
	}
}

void ULanguageOptionPanel::ApplySettingsFromUI(bool bSaveToDisk)
{
	Super::ApplySettingsFromUI(bSaveToDisk);
	
	const int32 SelectedIndex = OC_Language->GetCurrentIndex();
	
	if (SupportedCultures.IsValidIndex(SelectedIndex))
	{
		const FString NewCulture = SupportedCultures[SelectedIndex];
		UKismetInternationalizationLibrary::SetCurrentLanguage(NewCulture, bSaveToDisk);
	}
}

void ULanguageOptionPanel::ApplySettingsFromSavedData()
{
	Super::ApplySettingsFromSavedData();
	
	const FString CurrentLanguage = UKismetInternationalizationLibrary::GetCurrentLanguage();
	UKismetInternationalizationLibrary::SetCurrentLanguage(CurrentLanguage, false);
}

bool ULanguageOptionPanel::IsDirty() const
{
	if (Super::IsDirty())
	{
		return true;
	}
	
	// language apply immediately upon selection, it always returns false
	return false;
}

void ULanguageOptionPanel::OnLanguageRotated(int32 Value, ERotatorDirection RotatorDir)
{
	// if select a language, applied to the game immediately
	ApplySettingsFromUI(false);
}
