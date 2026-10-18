// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/GimmickSettingsCommon.h"
#include "Editor.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

UWorld* GimmickSettings::GetOpenLevelWorld()
{
	return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
}

bool GimmickSettings::IsPlayingInEditor()
{
	return GEditor && GEditor->PlayWorld != nullptr;
}

TSharedRef<IDetailsView> GimmickSettings::CreateDetailsView()
{
	FDetailsViewArgs DetailsArgs;
	DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsArgs.bAllowSearch = false;
	DetailsArgs.bHideSelectionTip = true;
	DetailsArgs.bUpdatesFromSelection = false;
	DetailsArgs.bLockable = false;
	DetailsArgs.bShowOptions = false;

	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	return PropertyEditor.CreateDetailView(DetailsArgs);
}
