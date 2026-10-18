// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/GimmickSettingsCommon.h"
#include "Editor.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

UWorld* GimmickSettings::GetCurrentLevelWorld()
{
	if (!GEditor) return nullptr;

	// The gimmicks run on the server, so a client world of PIE is never the one to show
	if (GEditor->PlayWorld)
	{
		for (const FWorldContext& Context : GEditor->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (Context.WorldType == EWorldType::PIE && World && World->GetNetMode() != NM_Client)
			{
				return World;
			}
		}
	}
	return GEditor->GetEditorWorldContext().World();
}

FString GimmickSettings::GetLevelPackageName(const UWorld& World)
{
	return UWorld::RemovePIEPrefix(World.GetPackage()->GetName());
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
