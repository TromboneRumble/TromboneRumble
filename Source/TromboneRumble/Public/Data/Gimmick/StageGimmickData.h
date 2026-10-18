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

	/** 기믹이 차례로 발동하는 순서. 기믹 목록에 넣은 이벤트형 기믹만 고를 수 있고, 마지막 기믹이 끝나면 처음부터 반복한다 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "시퀀스 순서", GetRestrictedEnumValues = "GetNonSequenceGimmickTypes"))
	TArray<EGimmickType> Order;

	/** 기믹이 켜진 뒤 첫 번째 기믹이 발동하기까지의 시간 (초) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "첫 시작 시간", ClampMin = "0.1", Units = "s"))
	float FirstStartTime = 20.f;

	/** 앞 기믹이 끝나고 다음 기믹이 발동하기까지의 시간 (초) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "기믹 간 발동 간격", ClampMin = "0.1", Units = "s"))
	float Gap = 5.f;

	/** 켜면 피버 타임부터 피버 타임 시 시퀀스 순서와 피버 타임 시 기믹 간 발동 간격을 쓴다 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "피버 타임 시 순서 변경"))
	bool bChangeOrderInFever = false;

	/** 피버 타임부터 쓰는 순서. 진행 중인 기믹은 끝까지 진행하고, 비우면 피버 동안 발동하지 않는다 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "피버 타임 시 시퀀스 순서", EditCondition = "bChangeOrderInFever", GetRestrictedEnumValues = "GetNonSequenceGimmickTypes"))
	TArray<EGimmickType> FeverOrder;

	/** 피버 타임에 앞 기믹이 끝나고 다음 기믹이 발동하기까지의 시간 (초) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "피버 타임 시 기믹 간 발동 간격", ClampMin = "0.1", Units = "s", EditCondition = "bChangeOrderInFever"))
	float FeverGap = 5.f;

	/** @return The order in use, which is FeverOrder in fever time when the sequence has one. */
	const TArray<EGimmickType>& GetOrder(const bool bFever) const { return bFever && bChangeOrderInFever ? FeverOrder : Order; }

	/** @return The gap in use, which is FeverGap in fever time when the sequence has a fever order. */
	float GetGap(const bool bFever) const { return bFever && bChangeOrderInFever ? FeverGap : Gap; }
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

	/** 이 레벨에서 쓰는 기믹의 설정. 종류마다 하나씩 넣는다. 레벨에 있는데 여기 없는 기믹은 기본값으로 동작한다 */
	UPROPERTY(EditAnywhere, Instanced, Category = "기믹", meta = (DisplayName = "기믹 목록", GetAllowedClasses = "GetAddableGimmickClasses"))
	TArray<TObjectPtr<UGimmickConfig>> Gimmicks;

	/** 여기 넣은 기믹은 자기 타이머 없이 차례로 돌아가며 발동한다. 자기 타이머가 없는 기믹(중력, 취객 등)은 여기에 있어야 발동한다 */
	UPROPERTY(EditAnywhere, Category = "기믹", meta = (DisplayName = "기믹 시퀀스"))
	TArray<FGimmickSequence> Sequences;

#if WITH_EDITOR
public:

	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	//~ End UObject Interface

private:

	/**
	 * Names of the gimmick types the order lists of a sequence cannot take. The details view greys them out.
	 * Only an event gimmick that is in the gimmick list can take turns.
	 */
	UFUNCTION()
	TArray<FString> GetNonSequenceGimmickTypes() const;

	/** Config classes whose type is not in the gimmick list yet. The class picker of the list shows only these. */
	UFUNCTION()
	TArray<UClass*> GetAddableGimmickClasses() const;
#endif
};
