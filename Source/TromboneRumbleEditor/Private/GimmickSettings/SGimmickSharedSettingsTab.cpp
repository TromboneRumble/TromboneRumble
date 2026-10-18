// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/SGimmickSharedSettingsTab.h"
#include "GimmickSettings/GimmickSettingsCommon.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "EngineUtils.h"
#include "IDetailRootObjectCustomization.h"
#include "IDetailsView.h"
#include "Styling/AppStyle.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FName GimmickSettingMetaKey(TEXT("GimmickSetting"));

	/** Bold title of one object in the tab. */
	TSharedRef<SWidget> MakeSectionTitle(const FText& Title)
	{
		return SNew(STextBlock)
			.Text(Title)
			.Font(FAppStyle::GetFontStyle("BoldFont"));
	}

	/**
	 * A Blueprint holds meshes, sounds, components and feel values too. Only properties tagged with GimmickSetting are shown.
	 * The test must not depend on the object. Component properties of an actor arrive with the component as their object,
	 * and an earlier version that let every object except the class defaults through showed the transform and the Niagara settings.
	 */
	bool IsGimmickSettingProperty(const FPropertyAndParent& PropertyAndParent)
	{
		if (PropertyAndParent.Property.HasMetaData(GimmickSettingMetaKey)) return true;

		for (const FProperty* Parent : PropertyAndParent.ParentProperties)
		{
			if (Parent && Parent->HasMetaData(GimmickSettingMetaKey)) return true;
		}
		return false;
	}

	/** Header of one object in the shared data tab. Shows the Blueprint or asset name instead of Default__X_C. */
	class FSharedDataHeaderCustomization : public IDetailRootObjectCustomization
	{
	public:

		//~ Begin IDetailRootObjectCustomization Interface
		virtual TSharedPtr<SWidget> CustomizeObjectHeader(const FDetailsObjectSet& InRootObjectSet, const TSharedPtr<ITableRow>& InTableRow) override
		{
			const UObject* Object = InRootObjectSet.RootObjects.Num() > 0 ? InRootObjectSet.RootObjects[0] : nullptr;
			if (!Object) return SNullWidget::NullWidget;

			const bool bIsClassDefaults = Object->HasAnyFlags(RF_ClassDefaultObject);
			return MakeSectionTitle(bIsClassDefaults ? Object->GetClass()->GetDisplayNameText() : FText::FromString(Object->GetName()));
		}

		virtual bool AreObjectsVisible(const FDetailsObjectSet& InRootObjectSet) const override { return true; }
		virtual bool ShouldDisplayHeader(const FDetailsObjectSet& InRootObjectSet) const override { return true; }
		virtual EExpansionArrowUsage GetExpansionArrowUsage() const override { return EExpansionArrowUsage::Default; }
		//~ End IDetailRootObjectCustomization Interface
	};
}

void SGimmickSharedSettingsTab::Construct(const FArguments& InArgs)
{
	DetailsView = GimmickSettings::CreateDetailsView(true);
	DetailsView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateStatic(&IsGimmickSettingProperty));
	// Rows added by detail customizations are not properties, so the filter above never sees them. Actors get several, such as the transform
	DetailsView->SetIsCustomRowVisibleDelegate(FIsCustomRowVisible::CreateSP(this, &SGimmickSharedSettingsTab::IsSettingCategory));
	DetailsView->SetRootObjectCustomizationInstance(MakeShared<FSharedDataHeaderCustomization>());

	ChildSlot
	[
		DetailsView.ToSharedRef()
	];
}

void SGimmickSharedSettingsTab::Rebuild(const UWorld* World)
{
	Objects.Reset();

	TArray<UObject*> ObjectsToShow;
	if (World)
	{
		for (TActorIterator<AGimmickBase> It(World); It; ++It)
		{
			TArray<UObject*> SettingsObjects;
			It->GetSettingsObjects(SettingsObjects);
			for (UObject* Object : SettingsObjects)
			{
				if (Object)
				{
					ObjectsToShow.AddUnique(Object);
				}
			}
		}
	}

	// The layout is built inside SetObjects and asks IsSettingCategory while it does, so the categories must be known first
	SettingCategories.Reset();
	for (UObject* Object : ObjectsToShow)
	{
		Objects.Add(Object);
		CollectSettingCategories(*Object->GetClass());
	}
	DetailsView->SetObjects(ObjectsToShow);
}

void SGimmickSharedSettingsTab::GetEditedPackages(TArray<UPackage*>& OutPackages) const
{
	for (const TWeakObjectPtr<UObject>& Object : Objects)
	{
		if (Object.IsValid())
		{
			OutPackages.AddUnique(Object->GetPackage());
		}
	}
}

void SGimmickSharedSettingsTab::CollectSettingCategories(const UClass& Class)
{
	for (TFieldIterator<FProperty> It(&Class); It; ++It)
	{
		if (!It->HasMetaData(GimmickSettingMetaKey)) continue;

		// The engine names each nested category node by its path so far (ObjectPropertyNode.cpp), so "Garbage|Config|Throw"
		// makes three nodes: "Garbage", "Garbage|Config" and "Garbage|Config|Throw". Every one of them is asked about,
		// and hiding any level hides everything below it. An earlier version added the single pieces but not the
		// middle path "Garbage|Config", which left the tab empty
		TArray<FString> Pieces;
		It->GetMetaData(TEXT("Category")).ParseIntoArray(Pieces, TEXT("|"), true);

		FString PathSoFar;
		for (const FString& Piece : Pieces)
		{
			if (!PathSoFar.IsEmpty())
			{
				PathSoFar += TEXT("|");
			}
			PathSoFar += Piece;
			SettingCategories.Add(FName(*PathSoFar));
		}
	}
}

bool SGimmickSharedSettingsTab::IsSettingCategory(const FName RowName, const FName ParentCategory) const
{
	// The engine asks this for custom rows and also for every subcategory. Returning false for all of them hid
	// the subcategories the settings live in, and the tab came up empty. The row name cannot tell the two apart,
	// because subcategories and most custom rows both pass None. The parent category can
	return SettingCategories.Contains(ParentCategory);
}
