// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DrunkardDespawn.generated.h"

/** UBTTask_DrunkardDespawn
 *
 * 퇴장 완료 지점(문 도착 후)에서 NPC를 제거한다. Server Only.
 * 스포너가 OnDestroyed를 구독하고 있어 반복 스폰 주기가 여기서부터 시작된다.
 */
UCLASS()
class TROMBONERUMBLE_API UBTTask_DrunkardDespawn : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_DrunkardDespawn();

	//~ Begin UBTTaskNode Interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	//~ End UBTTaskNode Interface
};
