// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GimmickSettings/SGimmickSettingsPanel.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "TromboneRumbleEditor"

namespace
{
	const FName GimmickSettingsTabName(TEXT("TromboneGimmickSettings"));

	TSharedRef<SDockTab> SpawnGimmickSettingsTab(const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SGimmickSettingsPanel)
			];
	}
}

/**
 * Editor only module of the project. It holds tools for designers and no game code.
 * Right now it registers the gimmick settings tab under the Tools menu.
 */
class FTromboneRumbleEditorModule : public IModuleInterface
{
public:

	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GimmickSettingsTabName, FOnSpawnTab::CreateStatic(&SpawnGimmickSettingsTab))
			.SetDisplayName(LOCTEXT("GimmickSettingsTabTitle", "기믹 설정"))
			.SetTooltipText(LOCTEXT("GimmickSettingsTabTooltip", "레벨의 기믹 수치를 한 곳에서 고치고 PIE에서 바로 테스트합니다"))
			.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
			.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
	}

	virtual void ShutdownModule() override
	{
		// Slate is already gone when the editor closes
		if (FSlateApplication::IsInitialized())
		{
			FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GimmickSettingsTabName);
		}
	}
	//~ End IModuleInterface Interface
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTromboneRumbleEditorModule, TromboneRumbleEditor)
