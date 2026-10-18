// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class IDetailsView;

/**
 * SGimmickSharedSettingsTab is the "투척물 종류별 수치" tab of the gimmick settings panel.
 * It shows the objects the gimmicks of the open level report through AGimmickBase::GetSettingsObjects.
 * Right now these are the garbage Blueprints, whose knockback and hit type differ per kind of garbage.
 * Only the properties tagged with GimmickSetting are shown, and each object is its own section.
 *
 * Keep in mind that it does not watch anything. SGimmickSettingsPanel calls Rebuild when the level changes.
 *
 * @see SGimmickSettingsPanel
 * @see AGimmickBase::GetSettingsObjects
 */
class SGimmickSharedSettingsTab : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SGimmickSharedSettingsTab) {}
	SLATE_END_ARGS()

	/** Build the details view and its filters. */
	void Construct(const FArguments& InArgs);

	/**
	 * Ask the gimmicks of the level for their objects again and show them.
	 *
	 * @param World Open level. The tab shows its objects even when the panel shows the asset of another level.
	 */
	void Rebuild(const UWorld* World);

	/** Add the package of every object shown, for the save button of the panel. */
	void GetEditedPackages(TArray<UPackage*>& OutPackages) const;

private:

	/** Add the category path of every GimmickSetting property of this class, and each shorter path above it. */
	void CollectSettingCategories(const UClass& Class);

	/** True when a custom row or a subcategory sits in a category that holds gimmick settings. */
	bool IsSettingCategory(FName RowName, FName ParentCategory) const;

	/** Details view of Objects. */
	TSharedPtr<IDetailsView> DetailsView;

	/** Objects shown. Filled by Rebuild. */
	TArray<TWeakObjectPtr<UObject>> Objects;

	/** Categories that hold GimmickSetting properties of the shown objects. Filled by Rebuild before the layout is built. */
	TSet<FName> SettingCategories;
};
