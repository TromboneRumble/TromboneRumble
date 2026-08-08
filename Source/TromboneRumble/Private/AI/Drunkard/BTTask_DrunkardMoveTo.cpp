// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "AI/Drunkard/BTTask_DrunkardMoveTo.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "AIController.h"
#include "Data/DrunkardDataAsset.h"

UBTTask_DrunkardMoveTo::UBTTask_DrunkardMoveTo()
{
	NodeName = TEXT("Move To (Drunkard)");
	// AcceptableRadius는 노드 공유 멤버라, 데이터 에셋 값으로 덮어쓰려면 트리별 인스턴스가 필요하다
	bCreateNodeInstance = true;

	// UE 5.5부터 에디터 노출이 제거된 플래그 (기본 꺼짐). 위빙 서비스가 MoveGoal(벡터 키)을
	// 갱신할 때마다 재경로하려면 필수 — 꺼져 있으면 최초 목표점으로만 이동한다
	bObserveBlackboardValue = true;
}

EBTNodeResult::Type UBTTask_DrunkardMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (const ADrunkardNPC* NPC = AIController ? Cast<ADrunkardNPC>(AIController->GetPawn()) : nullptr)
	{
		if (const UDrunkardDataAsset* Data = NPC->GetDrunkardData())
		{
			AcceptableRadius = Data->MoveAcceptanceRadius;
		}
	}

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
