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

void UGameplayOptionPanel::ReapplySavedSettings()
{
	Super::ReapplySavedSettings();
	
	if (!SaveManagerSubsystem)
	{
		return;
	}
	
	const FGameplaySettingData GameplayData = SaveManagerSubsystem->GetGameplaySettings();
	
	// TODO : 인게임 유저 닉네임 설정 & VOIP 설정 적용 
}

void UGameplayOptionPanel::HandleApplyButtonClicked()
{
	Super::HandleApplyButtonClicked();
	
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
	
	SaveManagerSubsystem->ApplyAndSaveGameplay(NewGameplayData);
}

void UGameplayOptionPanel::HandleResetButtonClicked()
{
	Super::HandleResetButtonClicked();
	
	RefreshUI();
	ReapplySavedSettings();
}
