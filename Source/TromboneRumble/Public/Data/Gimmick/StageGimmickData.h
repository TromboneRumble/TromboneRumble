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

	/** 기믹이 발동하는 순서. 마지막 기믹이 끝나면 처음부터 반복 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "순서"))
	TArray<EGimmickType> Order;

	/** 기믹이 켜진 뒤 첫 번째 기믹이 발동하기까지의 시간 (초) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "첫 발동 시간", ClampMin = "0.1", Units = "s"))
	float FirstDelay = 20.f;

	/** 앞 기믹이 끝나고 다음 기믹이 발동하기까지의 시간 (초) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "사이 간격", ClampMin = "0.1", Units = "s"))
	float Gap = 5.f;

	/** 켜면 피버 타임부터 피버 순서와 피버 사이 간격을 사용 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "피버 순서 사용"))
	bool bUseFeverOrder = false;

	/** 피버 타임부터 쓰는 순서. 진행 중인 기믹은 끝까지 진행하고, 비우면 피버 동안 발동하지 않음 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "피버 순서", EditCondition = "bUseFeverOrder"))
	TArray<EGimmickType> FeverOrder;

	/** 피버 타임에 앞 기믹이 끝나고 다음 기믹이 발동하기까지의 시간 (초) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "피버 사이 간격", ClampMin = "0.1", Units = "s", EditCondition = "bUseFeverOrder"))
	float FeverGap = 5.f;

	/** @return The order in use, which is FeverOrder in fever time when the sequence has one. */
	const TArray<EGimmickType>& GetOrder(const bool bFever) const { return bFever && bUseFeverOrder ? FeverOrder : Order; }

	/** @return The gap in use, which is FeverGap in fever time when the sequence has a fever order. */
	float GetGap(const bool bFever) const { return bFever && bUseFeverOrder ? FeverGap : Gap; }
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
