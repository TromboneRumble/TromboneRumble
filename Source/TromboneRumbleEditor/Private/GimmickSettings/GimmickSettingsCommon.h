// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IDetailsView;
class UWorld;

/** Helpers the widgets of the gimmick settings panel share. */
namespace GimmickSettings
{
	/** Left indent of the rows under a section header, so a header and its rows do not look alike. */
	constexpr float SectionRowIndent = 16.f;

	/**
	 * @return World of the current level: the server world of PIE while PIE runs, otherwise the level open in the editor.
	 *         A level change inside PIE moves it, while the editor world stays on the level PIE started from.
	 */
	UWorld* GetCurrentLevelWorld();

	/** @return Package name of the level of this world, without the prefix PIE adds, so a level has one name in the editor and in PIE. */
	FString GetLevelPackageName(const UWorld& World);

	/** @return Whether a PIE session runs. */
	bool IsPlayingInEditor();

	/** Make a details view for the panel, without the name area, the search box and the selection lock. */
	TSharedRef<IDetailsView> CreateDetailsView();
}
