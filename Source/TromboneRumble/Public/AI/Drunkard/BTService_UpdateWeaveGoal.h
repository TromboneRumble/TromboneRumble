// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateWeaveGoal.generated.h"

/** 위빙 서비스의 인스턴스별 상태 (BT 노드는 공유 객체라 멤버에 상태를 두면 안 됨) */
struct FBTWeaveGoalMemory
{
	float WeavePhase = 0.f;		// 사인파 초기 위상 (스폰마다 랜덤)
	float SpeedPhase = 0.f;
	float AmplitudeMul = 1.f;	// 현재 반주기의 진폭 배율 (기계적 사인파 방지용 변주)
	int32 LastHalfCycle = 0;
};

/** UBTService_UpdateWeaveGoal
 *
 * 취객 "갈지자 걸음"의 이동 레이어. Chasing 브랜치에 부착한다.
 * 타겟 위치에 진행 방향의 수직 사인파 오프셋을 더해 MoveGoal에 기록한다:
 *   MoveGoal = TargetLocation + Perpendicular × Amplitude × sin(Time × Frequency)
 * 진폭은 반주기마다 랜덤 변주되고, 이동 속도도 주기적으로 변주된다 (휘청→회복).
 * 수치는 전부 UDrunkardDataAsset에서 읽는다.
 */
UCLASS()
class TROMBONERUMBLE_API UBTService_UpdateWeaveGoal : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateWeaveGoal();

	//~ Begin UBTNode Interface
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTWeaveGoalMemory); }
	//~ End UBTNode Interface

protected:
	//~ Begin UBTAuxiliaryNode Interface
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	//~ End UBTAuxiliaryNode Interface

	/** 추격 대상 (Object: ADefaultTromboneCharacter) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	/** 위빙 보정이 적용된 이동 목표 (Vector) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveGoalKey;
};
