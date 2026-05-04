// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Settings/GameplayOptionPanel.h"
#include "SaveData/TromboneSaveGame.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/Common/CheckBoxRowWidget.h"
#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleRowWidget.h"
#include "Utilities/EnumHelper.h"

void UGameplayOptionPanel::RefreshUI()
{
	Super::RefreshUI();
	
	if (!SaveManagerSubsystem)
	{
		return;
	}
	
	const FGameplaySettingData GameplayData = SaveManagerSubsystem->GetGameplaySettings();
	
	CBR_ShouldShowUsernameInGame->SetChecked(GameplayData.bShouldShowUsernameInGame);
	OC_VOIP->SetSelectedIndex(EnumHelper::EnumToInt(GameplayData.VOIPSetting));
}

void UGameplayOptionPanel::ApplySettingsFromUI(bool bSaveToDisk)
{
	Super::ApplySettingsFromUI(bSaveToDisk);
	
	if (bSaveToDisk)
	{
		if (!SaveManagerSubsystem)
		{
			return;
		}
	
		VOIPType ParsedVoipType;
		if (!EnumHelper::IntToEnum(OC_VOIP->GetCurrentIndex(), ParsedVoipType))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid VOIP setting index: %d"), OC_VOIP->GetCurrentIndex());
			return;
		}
	
		FGameplaySettingData NewGameplayData;
		NewGameplayData.bShouldShowUsernameInGame = CBR_ShouldShowUsernameInGame->IsChecked();
		NewGameplayData.VOIPSetting = ParsedVoipType;
	
		SaveManagerSubsystem->ApplyGameplay(NewGameplayData, bSaveToDisk);
	}
	else
	{
		// TODO : 디스크에 저장하지 않고, 옵션 창에서 바로 보여줄 게 있으면 여기서 호출
	}
}

void UGameplayOptionPanel::ApplySettingsFromSavedData()
{
	Super::ApplySettingsFromSavedData();
	
	if (!SaveManagerSubsystem)
	{
		return;
	}
	
	const FGameplaySettingData GameplayData = SaveManagerSubsystem->GetGameplaySettings();
	
	// TODO : 옵션 창에서 바로 보여줄 게 있으면 여기서 호출
}

bool UGameplayOptionPanel::IsDirty() const
{
	if (Super::IsDirty())
	{
		return true;
	}

	if (!SaveManagerSubsystem)
	{
		return false;
	}

	const FGameplaySettingData SavedData = SaveManagerSubsystem->GetGameplaySettings();

	if (CBR_ShouldShowUsernameInGame && 
		CBR_ShouldShowUsernameInGame->IsChecked() != SavedData.bShouldShowUsernameInGame)
	{
		return true;
	}

	if (OC_VOIP)
	{
		VOIPType CurrentUISelectedVoip;
		if (EnumHelper::IntToEnum(OC_VOIP->GetCurrentIndex(), CurrentUISelectedVoip))
		{
			if (CurrentUISelectedVoip != SavedData.VOIPSetting)
			{
				return true;
			}
		}
	}

	return false;
}
