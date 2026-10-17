// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

/**
 * Draws a gimmick config inside the stage data list with its category groups.
 * The details view drops the categories of an object shown inline, so the settings of a gimmick would come as one flat list.
 * This groups them again by their Category meta, and "A|B" becomes the group B inside the group A.
 *
 * Keep in mind that a property without a category of its own stays flat, above the groups.
 * The top level category "고급" starts collapsed, for the settings a designer rarely touches.
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
