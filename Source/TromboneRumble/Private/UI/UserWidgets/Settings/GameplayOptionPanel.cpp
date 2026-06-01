// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Settings/GameplayOptionPanel.h"
#include "SaveData/TromboneSaveGame.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/Common/CheckBoxRowWidget.h"

void UGameplayOptionPanel::RefreshUI()
{
	Super::RefreshUI();

	if (!SaveManagerSubsystem)
	{
		return;
	}

	const FGameplaySettingData GameplayData = SaveManagerSubsystem->GetGameplaySettings();

	CBR_ShouldShowUsernameInGame->SetChecked(GameplayData.bShouldShowUsernameInGame);
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

		FGameplaySettingData NewGameplayData = SaveManagerSubsystem->GetGameplaySettings();
		NewGameplayData.bShouldShowUsernameInGame = CBR_ShouldShowUsernameInGame->IsChecked();
		SaveManagerSubsystem->ApplyGameplay(NewGameplayData, bSaveToDisk);
	}
}

void UGameplayOptionPanel::ApplySettingsFromSavedData()
{
	Super::ApplySettingsFromSavedData();

	if (!SaveManagerSubsystem)
	{
		return;
	}

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

	return false;
}
