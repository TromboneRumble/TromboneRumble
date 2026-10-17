// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/GimmickConfigCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "PropertyHandle.h"
#include "UObject/UnrealType.h"

namespace
{
	/** Top level category of the settings a designer rarely touches. Its group starts collapsed. */
	const FString AdvancedCategory(TEXT("고급"));

	/**
	 * @return Whether the property has a category of its own.
	 *         A property without one gets the class name as its category, and it stays out of the groups.
	 */
	bool ShouldGroup(const IPropertyHandle& Handle)
	{
		const FProperty* Property = Handle.GetProperty();
		const UClass* OwnerClass = Property ? Property->GetOwnerClass() : nullptr;
		return OwnerClass && Handle.GetMetaData(TEXT("Category")) != OwnerClass->GetName();
	}

	/**
	 * @return Whether EditConditionHides hides the row, so it must not open a group of its own.
	 *         Only a condition that names one bool property is read. Any other condition counts as met.
	 *         The row is left out instead of hidden, so this fits a condition that never changes in the editor, such as bSpawnableByManager.
	 */
	bool IsHiddenByEditCondition(const IPropertyHandle& Handle, const UObject& Config)
	{
		if (!Handle.HasMetaData(TEXT("EditConditionHides"))) return false;

		const FBoolProperty* Condition = FindFProperty<FBoolProperty>(Config.GetClass(), FName(*Handle.GetMetaData(TEXT("EditCondition"))));
		return Condition && !Condition->GetPropertyValue_InContainer(&Config);
	}

	/** Collect the property handles of the config, in the order the details view lists them. */
	void CollectPropertyHandles(const TSharedRef<IPropertyHandle>& Handle, TArray<TSharedRef<IPropertyHandle>>& OutHandles)
	{
		uint32 NumChildren = 0;
		Handle->GetNumChildren(NumChildren);

		for (uint32 Index = 0; Index < NumChildren; ++Index)
		{
			const TSharedPtr<IPropertyHandle> Child = Handle->GetChildHandle(Index);
			if (!Child.IsValid()) continue;

			// The inline object sits as one node without a property between the handle and the settings
			if (Child->GetProperty())
			{
				OutHandles.Add(Child.ToSharedRef());
			}
			else
			{
				CollectPropertyHandles(Child.ToSharedRef(), OutHandles);
			}
		}
	}
}

TSharedRef<IPropertyTypeCustomization> FGimmickConfigCustomization::MakeInstance()
{
	return MakeShared<FGimmickConfigCustomization>();
}

void FGimmickConfigCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// The default value widget keeps the class picker and the array element buttons
	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(250.f)
		[
			PropertyHandle->CreatePropertyValueWidget()
		];
}

void FGimmickConfigCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TArray<TSharedRef<IPropertyHandle>> Handles;
	CollectPropertyHandles(PropertyHandle, Handles);

	// Null when several stage data assets are selected and the entries differ. Every row is then added as before
	UObject* Config = nullptr;
	if (PropertyHandle->GetValue(Config) != FPropertyAccess::Success)
	{
		Config = nullptr;
	}

	// Keyed by the category path so far, so "A|B" and "C|B" stay two groups
	TMap<FString, IDetailGroup*> Groups;

	for (const TSharedRef<IPropertyHandle>& Handle : Handles)
	{
		// The row would be hidden, and its group would be left as an empty header
		if (Config && IsHiddenByEditCondition(*Handle, *Config)) continue;

		// A property without a category of its own, such as the drop class of a bonus, stays flat
		TArray<FString> Pieces;
		Handle->GetMetaData(TEXT("Category")).ParseIntoArray(Pieces, TEXT("|"), true);
		if (Pieces.IsEmpty() || !ShouldGroup(*Handle))
		{
			ChildBuilder.AddProperty(Handle);
			continue;
		}

		IDetailGroup* Group = nullptr;
		FString Path;
		for (const FString& Piece : Pieces)
		{
			Path = Path.IsEmpty() ? Piece : Path + TEXT("|") + Piece;

			if (IDetailGroup** Found = Groups.Find(Path))
			{
				Group = *Found;
				continue;
			}

			IDetailGroup* NewGroup = nullptr;
			if (Group)
			{
				NewGroup = &Group->AddGroup(FName(*Path), FText::FromString(Piece), true);
			}
			else
			{
				NewGroup = &ChildBuilder.AddGroup(FName(*Path), FText::FromString(Piece));
				NewGroup->ToggleExpansion(Piece != AdvancedCategory);
			}

			Groups.Add(Path, NewGroup);
			Group = NewGroup;
		}

		Group->AddPropertyRow(Handle);
	}
}
