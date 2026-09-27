// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utilities/Defines.h"
#include "StageGimmickData.generated.h"

class UGimmickConfig;

/**
 * Gimmicks that take turns. Each one starts only after the one before it has finished.
 * The manager runs it, and the gimmicks in it do not start themselves.
 */
USTRUCT(BlueprintType)
struct FGimmickSequence
{
	GENERATED_BODY()

	/** Gimmicks in the order they run. After the last one the order starts over. */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "순서"))
	TArray<EGimmickType> Order;

	/** Seconds from the start of the gimmicks to the first one in the order. */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "첫 발동 시간", ClampMin = "0.0", Units = "s"))
	float FirstDelay = 20.f;

	/** Seconds from the end of one gimmick to the start of the next one. */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "사이 간격", ClampMin = "0.0", Units = "s"))
	float Gap = 5.f;
};

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

	/** @return The sequence this gimmick type belongs to, or null when it runs on its own timer. */
	const FGimmickSequence* FindSequence(EGimmickType GimmickType) const;

	const TArray<FGimmickSequence>& GetSequences() const { return Sequences; }

private:

	/** One entry per gimmick used in the level. Keep one entry per type. */
	UPROPERTY(EditAnywhere, Instanced, Category = "Gimmick", meta = (DisplayName = "기믹 목록"))
	TArray<TObjectPtr<UGimmickConfig>> Gimmicks;

	/** Gimmicks listed here take turns instead of running on their own timers. */
	UPROPERTY(EditAnywhere, Category = "Gimmick", meta = (DisplayName = "순서 그룹"))
	TArray<FGimmickSequence> Sequences;

#if WITH_EDITOR
public:

	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	//~ End UObject Interface
#endif
};
