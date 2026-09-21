// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utilities/Defines.h"
#include "StageGimmickData.generated.h"

class UGimmickConfig;

/**
 * UStageGimmickData holds the gimmick configs of one level. Name the asset DA_Gimmicks_<Level>.
 * The AGimmickManager placed in the level points at it.
 * A gimmick that is not in the list runs with the defaults of its config class.
 *
 * @see UGimmickConfig
 * @see AGimmickManager
 */
UCLASS()
class TROMBONERUMBLE_API UStageGimmickData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** @return The config for this gimmick type, or null when the level has no entry for it. */
	const UGimmickConfig* FindConfig(EGimmickType GimmickType) const;

	/** @return Every entry of the level. The gimmick settings panel lists them. */
	const TArray<TObjectPtr<UGimmickConfig>>& GetGimmicks() const { return Gimmicks; }

private:

	/** One entry per gimmick used in the level. Keep one entry per type. */
	UPROPERTY(EditAnywhere, Instanced, Category = "Gimmick", meta = (DisplayName = "기믹 목록"))
	TArray<TObjectPtr<UGimmickConfig>> Gimmicks;

#if WITH_EDITOR
public:

	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	//~ End UObject Interface
#endif
};
