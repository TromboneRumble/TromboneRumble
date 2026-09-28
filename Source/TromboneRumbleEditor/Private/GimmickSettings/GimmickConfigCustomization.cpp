// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/GimmickConfigCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "PropertyHandle.h"

namespace
{
	/** Class meta that turns the category groups on for the properties that class declares. */
	const FName GroupSettingsByCategoryMetaKey(TEXT("GroupSettingsByCategory"));

	/** @return Whether the class that declares this property asked for category groups. */
	bool ShouldGroup(const IPropertyHandle& Handle)
	{
		const FProperty* Property = Handle.GetProperty();
		const UClass* OwnerClass = Property ? Property->GetOwnerClass() : nullptr;
		return OwnerClass && OwnerClass->HasMetaData(GroupSettingsByCategoryMetaKey);
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

	// Keyed by the category path so far, so "A|B" and "C|B" stay two groups
	TMap<FString, IDetailGroup*> Groups;

	for (const TSharedRef<IPropertyHandle>& Handle : Handles)
	{
		// A property of a base class that did not opt in, such as the schedule, stays flat
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
				NewGroup->ToggleExpansion(true);
			}

			Groups.Add(Path, NewGroup);
			Group = NewGroup;
		}

		Group->AddPropertyRow(Handle);
	}
}
