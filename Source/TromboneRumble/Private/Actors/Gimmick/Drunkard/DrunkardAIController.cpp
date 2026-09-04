// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Drunkard/DrunkardAIController.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "Actors/Gimmick/Drunkard/DrunkardSpawner.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/DefaultTromboneCharacter.h"

const FName ADrunkardAIController::BBKeyTargetPlayer(TEXT("TargetPlayer"));
const FName ADrunkardAIController::BBKeyMoveGoal(TEXT("MoveGoal"));
const FName ADrunkardAIController::BBKeyState(TEXT("State"));
const FName ADrunkardAIController::BBKeyExitDoor(TEXT("ExitDoor"));

void ADrunkardAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}

	StateComponent = InPawn ? InPawn->FindComponentByClass<UDrunkardStateComponent>() : nullptr;
	if (StateComponent)
	{
		StateComponent->OnStateChanged.AddDynamic(this, &ThisClass::HandleStateChanged);
		StateComponent->OnTargetChanged.AddDynamic(this, &ThisClass::HandleTargetChanged);

		// 빙의 전에 일어난 전이를 반영 (스포너의 BeginEntering이 먼저 호출됐을 수 있음)
		HandleStateChanged(StateComponent->GetState());
		HandleTargetChanged(StateComponent->GetTarget());
	}
}

void ADrunkardAIController::OnUnPossess()
{
	if (StateComponent)
	{
		StateComponent->OnStateChanged.RemoveDynamic(this, &ThisClass::HandleStateChanged);
		StateComponent->OnTargetChanged.RemoveDynamic(this, &ThisClass::HandleTargetChanged);
		StateComponent = nullptr;
	}

	Super::OnUnPossess();
}

void ADrunkardAIController::HandleStateChanged(const EDrunkardState NewState)
{
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
	if (!BlackboardComp) return;

	BlackboardComp->SetValueAsEnum(BBKeyState, static_cast<uint8>(NewState));

	// 퇴장할 문 = 종료 시점 최근접 문
	if (NewState == EDrunkardState::Exiting)
	{
		if (const ADrunkardNPC* NPC = Cast<ADrunkardNPC>(GetPawn()))
		{
			if (const ADrunkardSpawner* Spawner = NPC->GetOwningSpawner())
			{
				BlackboardComp->SetValueAsObject(BBKeyExitDoor, Spawner->FindClosestDoor(NPC->GetActorLocation()));
			}
		}
	}
}

void ADrunkardAIController::HandleTargetChanged(ADefaultTromboneCharacter* NewTarget)
{
	if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
	{
		BlackboardComp->SetValueAsObject(BBKeyTargetPlayer, NewTarget);
	}
}
