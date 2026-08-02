// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Data/XRayFadeMaterialMap.h"
#include "Materials/MaterialInterface.h"

UMaterialInterface* UXRayFadeMaterialMap::FindFadeVariant(UMaterialInterface* Original) const
{
	if (!Original)
	{
		return nullptr;
	}

	const TObjectPtr<UMaterialInterface>* Found = FadeVariants.Find(Original);
	return Found ? Found->Get() : nullptr;
}

void UXRayFadeMaterialMap::GetAllFadeVariants(TArray<UMaterialInterface*>& OutVariants) const
{
	OutVariants.Reserve(OutVariants.Num() + FadeVariants.Num());

	for (const TPair<TObjectPtr<UMaterialInterface>, TObjectPtr<UMaterialInterface>>& Pair : FadeVariants)
	{
		if (UMaterialInterface* Variant = Pair.Value.Get())
		{
			OutVariants.Add(Variant);
		}
	}
}
