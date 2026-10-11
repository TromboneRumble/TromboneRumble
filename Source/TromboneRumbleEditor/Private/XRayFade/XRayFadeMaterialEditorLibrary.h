// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XRayFadeMaterialEditorLibrary.generated.h"

class UMaterial;

/**
 * Material edits for Tools/generate_xray_fade_materials.py that Python cannot do on its own.
 * A root pin in constant mode compiles its constant and ignores the connected node, and only the material editor graph can turn it off.
 * Copies of such materials would keep Opacity at 1.0 even when the fade function is wired in.
 */
UCLASS()
class UXRayFadeMaterialEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Sets the constant mode of the root Opacity pin. Pass false so the connected node is compiled. */
	UFUNCTION(BlueprintCallable, Category = "XRay|Editor")
	static void SetOpacityUseConstant(UMaterial* Material, bool bUseConstant);

	/** @return True if the root Opacity pin compiles its constant instead of the connected node. */
	UFUNCTION(BlueprintPure, Category = "XRay|Editor")
	static bool GetOpacityUseConstant(const UMaterial* Material);
};
