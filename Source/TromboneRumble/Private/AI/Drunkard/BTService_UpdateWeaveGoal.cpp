// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "AI/Drunkard/BTService_UpdateWeaveGoal.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "Components/ActorComponents/DrunkardStateComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Data/DrunkardDataAsset.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

UBTService_UpdateWeaveGoal::UBTService_UpdateWeaveGoal()
{
	NodeName = TEXT("Update Weave Goal");
	Interval = 0.1f;
	RandomDeviation = 0.f;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	// 브랜치 진입 즉시 첫 틱을 돌려 MoveTo가 실행되기 전에 MoveGoal이 채워지게 한다
	bCallTickOnSearchStart = true;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateWeaveGoal, TargetPlayerKey), AActor::StaticClass());
	MoveGoalKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateWeaveGoal, MoveGoalKey));
}

void UBTService_UpdateWeaveGoal::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	// 키 셀렉터는 노드가 직접 블랙보드 에셋에 연결해야 KeyID가 유효해진다 (엔진 BTTask_BlackboardBase 패턴)
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetPlayerKey.ResolveSelectedKey(*BBAsset);
		MoveGoalKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_UpdateWeaveGoal::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	FBTWeaveGoalMemory* Memory = CastInstanceNodeMemory<FBTWeaveGoalMemory>(NodeMemory);
	Memory->WeavePhase = FMath::FRandRange(0.f, 2.f * PI);
	Memory->SpeedPhase = FMath::FRandRange(0.f, 2.f * PI);
	Memory->AmplitudeMul = 1.f;
	Memory->LastHalfCycle = TNumericLimits<int32>::Min();
}

void UBTService_UpdateWeaveGoal::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 변주된 이동 속도를 기본값으로 복원.
	// 단, 피격 경직 중이면 복원하지 않는다 — 경직이 속도를 0으로 잠근 상태를 브랜치 이탈이
	// 덮어쓰면 스턴 중에 퇴장 이동을 시작해 버린다 (경직 해제 시 HandleStunStateChanged가 복원)
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (const ADrunkardNPC* NPC = AIController ? Cast<ADrunkardNPC>(AIController->GetPawn()) : nullptr)
	{
		const UDrunkardDataAsset* Data = NPC->GetDrunkardData();
		if (UCharacterMovementComponent* Move = NPC->GetCharacterMovement(); Move && Data && !NPC->IsStun())
		{
			Move->MaxWalkSpeed = Data->WalkSpeed;
		}
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_UpdateWeaveGoal::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const AAIController* AIController = OwnerComp.GetAIOwner();
	const ADrunkardNPC* NPC = AIController ? Cast<ADrunkardNPC>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!NPC || !Blackboard) return;

	const UDrunkardDataAsset* Data = NPC->GetDrunkardData();
	if (!Data) return;

	// 피격 경직 중에는 위빙/속도 변주를 멈춘다 (HandleStunStateChanged가 속도를 0으로 잠근 상태를 덮어쓰지 않도록)
	if (NPC->IsStun()) return;

	const AActor* Target = Cast<AActor>(Blackboard->GetValue<UBlackboardKeyType_Object>(TargetPlayerKey.GetSelectedKeyID()));
	if (!Target)
	{
		// 타겟이 사라짐(세션 이탈 등) — 재선정 요청. 후보가 없으면 다음 틱에 다시 시도된다
		if (UDrunkardStateComponent* StateComponent = NPC->GetStateComponent())
		{
			StateComponent->RequestTargetChange();
		}
		return;
	}

	FBTWeaveGoalMemory* Memory = CastInstanceNodeMemory<FBTWeaveGoalMemory>(NodeMemory);
	const float Time = NPC->GetWorld()->GetTimeSeconds();
	const float SinArg = Time * Data->WeaveFrequency + Memory->WeavePhase;

	// 반주기(방향이 바뀌는 지점)마다 진폭 배율을 재롤 — 기계적인 사인파로 보이는 것을 방지
	const int32 HalfCycle = FMath::FloorToInt32(SinArg / PI);
	if (HalfCycle != Memory->LastHalfCycle)
	{
		Memory->LastHalfCycle = HalfCycle;
		Memory->AmplitudeMul = FMath::FRandRange(1.f - Data->WeaveAmplitudeNoise, 1.f + Data->WeaveAmplitudeNoise);
	}

	const ATromboneCharacterBase* TargetCharacter = Cast<ATromboneCharacterBase>(Target);
	FVector TargetLocation = TargetCharacter ? TargetCharacter->GetPelvisLocation() : Target->GetActorLocation();

	// 골반은 뼈 위치라 그대로 넘기면 안 된다. 도달 판정(HasReachedInternal)은 목표와 폰 "중심"의 Z 차이를
	// 캡슐 절반 높이 기준으로 검사하는데, 쓰러진 몸의 골반은 지면에 붙어 있어 그 한계를 넘겨 영영 도달로 인정되지 않는다.
	// 골반이 선 지면을 내비메시에서 찾아 폰 중심 높이로 올려주면, 층 정보는 타겟 기준으로 유지하면서 Z 차이가 사라진다
	if (const UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(NPC->GetWorld()))
	{
		FNavLocation ProjectedLocation;
		if (NavSystem->ProjectPointToNavigation(TargetLocation, ProjectedLocation))
		{
			TargetLocation.Z = ProjectedLocation.Location.Z + NPC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}
	}
	FVector ToTarget = TargetLocation - NPC->GetActorLocation();
	ToTarget.Z = 0.f;
	const float Distance = ToTarget.Size();

	FVector MoveGoal = TargetLocation;
	if (Distance > KINDA_SMALL_NUMBER)
	{
		ToTarget /= Distance;
		const FVector Perpendicular = FVector::CrossProduct(ToTarget, FVector::UpVector);

		// 타겟에 가까워지면 오프셋을 줄인다 — 목표가 계속 옆으로 흔들리면 접촉(포획 판정)이 어려워짐
		const float ProximityDamp = FMath::Min(1.f, Distance / FMath::Max(Data->WeaveAmplitude * 2.f, KINDA_SMALL_NUMBER));
		const float Offset = Data->WeaveAmplitude * Memory->AmplitudeMul * ProximityDamp * FMath::Sin(SinArg);
		MoveGoal += Perpendicular * Offset;
	}
	Blackboard->SetValue<UBlackboardKeyType_Vector>(MoveGoalKey.GetSelectedKeyID(), MoveGoal);

	// 이동 속도 주기적 변주 (휘청→회복). 위빙과 다른 주기로 돌려 패턴이 겹쳐 보이지 않게 한다
	if (Data->SpeedVariance > 0.f)
	{
		if (UCharacterMovementComponent* Move = NPC->GetCharacterMovement())
		{
			const float SpeedSin = FMath::Sin(Time * Data->WeaveFrequency * 0.7f + Memory->SpeedPhase);
			Move->MaxWalkSpeed = Data->WalkSpeed * (1.f + Data->SpeedVariance * SpeedSin);
		}
	}
}
