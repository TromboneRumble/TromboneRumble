// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "AI/Drunkard/BTTask_DrunkardMoveTo.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "AIController.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/ActorComponents/DrunkardStateComponent.h"
#include "Data/DrunkardDataAsset.h"

UBTTask_DrunkardMoveTo::UBTTask_DrunkardMoveTo()
{
	NodeName = TEXT("Move To (Drunkard)");
	// AcceptableRadius는 노드 공유 멤버라, 데이터 에셋 값으로 덮어쓰려면 트리별 인스턴스가 필요하다
	bCreateNodeInstance = true;

	// UE 5.5부터 에디터 노출이 제거된 플래그 (기본 꺼짐). 위빙 서비스가 MoveGoal(벡터 키)을
	// 갱신할 때마다 재경로하려면 필수 — 꺼져 있으면 최초 목표점으로만 이동한다
	bObserveBlackboardValue = true;

	// OnTaskFinished 통지를 받기 위해 필요 (파생 클래스마다 각자 호출해야 한다)
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

void UBTTask_DrunkardMoveTo::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	// If target cannot be hit, switch targets.
	if (TaskResult != EBTNodeResult::Succeeded) return;

	const AAIController* AIController = OwnerComp.GetAIOwner();
	const ADrunkardNPC* NPC = AIController ? Cast<ADrunkardNPC>(AIController->GetPawn()) : nullptr;
	UDrunkardStateComponent* StateComponent = NPC ? NPC->GetStateComponent() : nullptr;
	if (!StateComponent) return;

	const ADefaultTromboneCharacter* TargetCharacter = StateComponent->GetTarget();
	if (TargetCharacter && !TargetCharacter->CanReceiveHit())
	{
		StateComponent->RequestTargetChange();
	}
}

EBTNodeResult::Type UBTTask_DrunkardMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (const ADrunkardNPC* NPC = AIController ? Cast<ADrunkardNPC>(AIController->GetPawn()) : nullptr)
	{
		if (const UDrunkardDataAsset* Data = NPC->GetDrunkardData())
		{
			AcceptableRadius = Data->MoveAcceptanceRadius;

			// 허용 오차는 생성자에서 엔진 기본 반경 기준으로 잡히므로 함께 갱신한다.
			// 이 값보다 목표가 크게 움직이면 이동 요청이 취소·재발급되어 도달이 미뤄진다
			ObservedBlackboardValueTolerance = Data->MoveAcceptanceRadius * 0.95f;
		}
	}

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
