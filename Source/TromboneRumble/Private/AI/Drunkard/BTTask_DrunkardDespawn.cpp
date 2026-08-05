// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "AI/Drunkard/BTTask_DrunkardDespawn.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "Components/ActorComponents/DrunkardStateComponent.h"
#include "AIController.h"

UBTTask_DrunkardDespawn::UBTTask_DrunkardDespawn()
{
	NodeName = TEXT("Despawn Drunkard");
}

EBTNodeResult::Type UBTTask_DrunkardDespawn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const ADrunkardNPC* NPC = AIController ? Cast<ADrunkardNPC>(AIController->GetPawn()) : nullptr;
	if (!NPC || !NPC->HasAuthority())
	{
		return EBTNodeResult::Failed;
	}

	UDrunkardStateComponent* StateComponent = NPC->GetStateComponent();
	if (!StateComponent)
	{
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Log, TEXT("[Drunkard] %s 문 도달 — 디스폰"), *NPC->GetName());
	StateComponent->DespawnOwner();

	return EBTNodeResult::Succeeded;
}
