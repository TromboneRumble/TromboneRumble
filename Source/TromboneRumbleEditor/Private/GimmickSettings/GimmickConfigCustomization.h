// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

/**
 * Draws a gimmick config inside the stage data list with its category groups.
 * The details view drops the categories of an object shown inline, so the settings of a gimmick would come as one flat list.
 * This groups them again by their Category meta, and "A|B" becomes the group B inside the group A.
 *
 * Keep in mind that only the properties of a class with the GroupSettingsByCategory class meta are grouped.
 * Every other property stays flat as before, so configs that have not opted in look the same.
 *
 * @see UGimmickConfig
 */
class FGimmickConfigCustomization : public IPropertyTypeCustomization
{
public:

	/** @return A new instance, for the property editor module. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface
};
