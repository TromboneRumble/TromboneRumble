// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "XRayFade/XRayFadeMaterialEditorLibrary.h"
#include "Materials/Material.h"

void UXRayFadeMaterialEditorLibrary::SetOpacityUseConstant(UMaterial* Material, bool bUseConstant)
{
	UMaterialEditorOnlyData* EditorOnlyData = Material ? Material->GetEditorOnlyData() : nullptr;
	if (!EditorOnlyData)
	{
		return;
	}

	Material->Modify();
	EditorOnlyData->Opacity.UseConstant = bUseConstant;
}

bool UXRayFadeMaterialEditorLibrary::GetOpacityUseConstant(const UMaterial* Material)
{
	const UMaterialEditorOnlyData* EditorOnlyData = Material ? Material->GetEditorOnlyData() : nullptr;
	return EditorOnlyData && EditorOnlyData->Opacity.UseConstant;
}
