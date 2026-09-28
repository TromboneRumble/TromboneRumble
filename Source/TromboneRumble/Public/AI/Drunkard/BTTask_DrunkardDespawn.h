// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DrunkardDespawn.generated.h"

/** UBTTask_DrunkardDespawn
 *
 * 퇴장 중 정지 지점에 도착하면 문 밖 이동을 시작한다. Server Only.
 * NPC가 생성 지점까지 콜리전 없이 이동한 뒤 소멸하고, BT는 그동안 멈춘다.
 * 에셋이 이 클래스를 참조하고 있어 이름은 그대로 둔다.
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
