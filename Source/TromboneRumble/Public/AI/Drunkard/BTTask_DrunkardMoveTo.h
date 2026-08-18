// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_DrunkardMoveTo.generated.h"

/** UBTTask_DrunkardMoveTo
 *
 * 표준 MoveTo에서 도달 판정 반경만 UDrunkardDataAsset 값으로 덮어쓴다.
 * 반경이 크면 목표 근처에서 위빙이 죽기 때문에 기획자가 데이터 에셋에서 튜닝한다.
 */
UCLASS()
class TROMBONERUMBLE_API UBTTask_DrunkardMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_DrunkardMoveTo();

	//~ Begin UBTTaskNode Interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	//~ End UBTTaskNode Interface
};
