// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "Actors/Gimmick/Drunkard/DrunkardAIController.h"
#include "Actors/Gimmick/Drunkard/DrunkardSpawner.h"
#include "Components/ActorComponents/DrunkardStateComponent.h"
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Components/ActorComponents/XRaySilhouetteComponent.h"
#include "Data/DrunkardDataAsset.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogDrunkard);

#if !UE_BUILD_SHIPPING
// 취객 기믹 전 상태 관찰용. 스포너(DrunkardSpawner.cpp)에서도 extern으로 참조
TAutoConsoleVariable<int32> CVarDrunkardDebug(
	TEXT("Trombone.Drunkard.Debug"),
	0,
	TEXT("1이면 취객 NPC 기믹의 상태를 화면과 월드에 표시"));
#endif

ADrunkardNPC::ADrunkardNPC()
{
	AIControllerClass = ADrunkardAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 오프스크린(문 뒤)에서 스폰·이동하므로 렌더링 여부와 무관하게 포즈를 평가해야 한다.
	// 기본값(OnlyTickPoseWhenRendered)이면 포즈 미평가로 트랜스폼 버퍼가 비어,
	// 상체 물리(PhysicalAnimation)가 빈 버퍼에 접근해 크래시한다
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	StateComponent = CreateDefaultSubobject<UDrunkardStateComponent>(TEXT("DrunkardStateComponent"));
	XRaySilhouetteComponent = CreateDefaultSubobject<UXRaySilhouetteComponent>(TEXT("XRaySilhouetteComponent"));
}

void ADrunkardNPC::BeginPlay()
{
	Super::BeginPlay();

	if (DrunkardData)
	{
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->MaxWalkSpeed = DrunkardData->WalkSpeed;
			Move->MaxAcceleration = DrunkardData->MaxAcceleration;
		}
	}

	// X-Ray 토글이 꺼져 있으면 컴포넌트 제거 (안 쓰는 CustomDepth 비용 방지)
	if (XRaySilhouetteComponent && (!DrunkardData || !DrunkardData->bEnableXRaySilhouette))
	{
		XRaySilhouetteComponent->DestroyComponent();
		XRaySilhouetteComponent = nullptr;
	}

	// 래그돌이 끝나 메시 물리가 리셋되면 상체 물리를 다시 얹는다
	if (RagdollComponent)
	{
		RagdollComponent->OnRagdollPhysicsEnabled.AddDynamic(this, &ThisClass::ApplyUpperBodyPhysics);
	}

	// 피격 경직(스턴) 동안 이동 정지 (스턴 애니메이션은 ABP의 bIsStunned가 자동 처리)
	OnStunStateChanged.AddDynamic(this, &ThisClass::HandleStunStateChanged);

	// 스폰 프레임에는 첫 포즈 평가 전이라 본 트랜스폼 버퍼가 완성되지 않았을 수 있다.
	// 그 상태로 PhysicalAnimation 제약이 생성되면 엔진이 빈 버퍼에 무검증 접근해 크래시하므로 한 틱 미룬다
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::ApplyUpperBodyPhysics);
}

void ADrunkardNPC::ApplyUpperBodyPhysics()
{
	if (!DrunkardData || !DrunkardData->bEnableUpperBodyPhysics) return;

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || !PhysicalAnimationComp) return;

	if (IsRagdoll()) return;

	// PhysicalAnimationComponent는 제약 생성 시 메시의 트랜스폼 버퍼에 본 인덱스로 무검증 접근한다
	// (ComputeWorldSpaceTargetTM). 버퍼가 비는 구성(메시 에셋 없음, 리더 포즈 follower)에서는
	// 어설션 크래시가 나므로 적용 전에 걸러낸다
	if (!MeshComp->GetSkeletalMeshAsset() || !MeshComp->GetPhysicsAsset()) return;
	if (MeshComp->LeaderPoseComponent.IsValid())
	{
		UE_LOG(LogDrunkard, Warning, TEXT("%s: 메시가 리더 포즈 follower라 상체 물리를 적용할 수 없음"), *GetName());
		return;
	}

	// 편집용/읽기용 본 트랜스폼 버퍼가 모두 준비될 때까지 적용을 미룬다 (첫 포즈 평가 후 채워짐).
	// 계속 비어 있다면 메시가 포즈를 평가하지 않는 것 — VisibilityBasedAnimTickOption 확인
	if (MeshComp->GetEditableComponentSpaceTransforms().IsEmpty() || MeshComp->GetComponentSpaceTransforms().IsEmpty())
	{
		constexpr int32 MaxRetries = 60;
		if (++UpperBodyPhysicsRetryCount <= MaxRetries)
		{
			UE_LOG(LogDrunkard, Warning, TEXT("%s: 본 트랜스폼 버퍼 미준비 (editable=%d, read=%d, 시도 %d) — 상체 물리 적용 연기"),
				*GetName(), MeshComp->GetEditableComponentSpaceTransforms().Num(), MeshComp->GetComponentSpaceTransforms().Num(), UpperBodyPhysicsRetryCount);
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::ApplyUpperBodyPhysics);
		}
		else
		{
			UE_LOG(LogDrunkard, Error, TEXT("%s: 본 트랜스폼 버퍼가 %d틱 동안 준비되지 않음 — 상체 물리 포기. 메시의 VisibilityBasedAnimTickOption이 AlwaysTickPoseAndRefreshBones인지 확인 필요"),
				*GetName(), MaxRetries);
		}
		return;
	}
	UpperBodyPhysicsRetryCount = 0;
	if (MeshComp->GetBoneIndex(DrunkardData->UpperBodyPhysicsRootBone) == INDEX_NONE)
	{
		UE_LOG(LogDrunkard, Warning, TEXT("%s: 상체 물리 시작 본 '%s' 이 스켈레톤에 없음 — 상체 물리 미적용"),
			*GetName(), *DrunkardData->UpperBodyPhysicsRootBone.ToString());
		return;
	}

	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// 자기 캡슐의 이동 스윕이 자기 시뮬 바디에 막히지 않도록 제외 (동일 액터의 컴포넌트 간 물리 충돌은 자동 제외되지 않는다)
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->IgnoreComponentWhenMoving(MeshComp, true);
	}

	// 하반신(지정 본 위쪽 계층)은 애니메이션 유지 — 전신에 걸면 메시가 캡슐에서 이탈해 포획 판정이 거짓말을 하게 된다
	MeshComp->SetAllBodiesBelowSimulatePhysics(DrunkardData->UpperBodyPhysicsRootBone, true, true);
	PhysicalAnimationComp->ApplyPhysicalAnimationSettingsBelow(DrunkardData->UpperBodyPhysicsRootBone, DrunkardData->UpperBodyPhysicsProfile, true);

	// 피직스 에셋 바디가 자체 콜리전 프로파일(Ragdoll 등, Pawn을 Block)을 쓰면 위의 컴포넌트 레벨
	// 응답 설정이 무시된다. 시뮬 상체가 자기/타 캐릭터의 캡슐 이동을 막지 않도록 바디 단위로 강제
	MeshComp->ForEachBodyBelow(DrunkardData->UpperBodyPhysicsRootBone, true, false,
		[](FBodyInstance* Body)
		{
			Body->SetResponseToChannel(ECC_Pawn, ECR_Ignore);
		});
}

void ADrunkardNPC::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if !UE_BUILD_SHIPPING
	if (CVarDrunkardDebug.GetValueOnGameThread() != 0)
	{
		DebugDrawGimmickState();
	}
#endif
}

void ADrunkardNPC::DebugDrawGimmickState() const
{
#if !UE_BUILD_SHIPPING
	if (!GEngine || !StateComponent) return;

	const EDrunkardState State = StateComponent->GetState();
	const ADefaultTromboneCharacter* Target = StateComponent->GetTarget();

	const AAIController* AIController = Cast<AAIController>(GetController());
	const UBlackboardComponent* Blackboard = AIController ? AIController->GetBlackboardComponent() : nullptr;
	const FVector MoveGoal = Blackboard ? Blackboard->GetValueAsVector(ADrunkardAIController::BBKeyMoveGoal) : FVector::ZeroVector;

	// 상태/타겟은 서버 전용이라 리슨서버 호스트 화면 기준으로 관찰한다 (클라 화면에서는 None으로 보임)
	const UCharacterMovementComponent* Move = GetCharacterMovement();

	TStringBuilder<512> Text;
	Text.Appendf(TEXT("── 취객 %s [%s] ──\n"), *GetName(), HasAuthority() ? TEXT("서버") : TEXT("클라"));

	// 상태
	FString Remaining;
	switch (State)
	{
		case EDrunkardState::Entering:
			Remaining = FString::Printf(TEXT("  추격까지 %.1fs"), FMath::Max(0.f, StateComponent->GetRemainingEnterTime()));
			break;
		case EDrunkardState::Chasing:
			Remaining = FString::Printf(TEXT("  퇴장까지 %.1fs"), FMath::Max(0.f, StateComponent->GetRemainingChaseTime()));
			break;
		default:
			break;
	}
	Text.Appendf(TEXT("상태   %s%s\n"), *UEnum::GetDisplayValueAsText(State).ToString(), *Remaining);

	// 자신
	Text.Appendf(TEXT("자신   속도 %.0f/%.0f   피격 %s%s\n"),
		Move ? Move->Velocity.Size2D() : 0.f,
		Move ? Move->MaxWalkSpeed : 0.f,
		CanReceiveHit() ? TEXT("가능") : TEXT("불가"),
		CanReceiveHit() ? TEXT("") : (IsStun() ? TEXT("(스턴)") : IsRagdoll() ? TEXT("(래그돌)") : TEXT("(무적)")));

	// 타겟
	if (!Target)
	{
		Text.Append(TEXT("타겟   없음\n"));
	}
	else
	{
		Text.Appendf(TEXT("타겟   %s   %s\n"), *Target->GetName(), Target->CanReceiveHit() ? TEXT("포획가능") : TEXT("포획불가"));
		Text.Appendf(TEXT("거리   %.0fcm\n"), FVector::Dist2D(GetActorLocation(), Target->GetPelvisLocation()));
	}

	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 1.f, FColor::Yellow, Text.ToString());

	// 월드 시각화 — 노랑: 위빙 반영 이동 목표, 초록: 타겟, 하늘색: 퇴장 문
	if (Blackboard && State == EDrunkardState::Chasing)
	{
		DrawDebugSphere(GetWorld(), MoveGoal, 20.f, 8, FColor::Yellow, false, -1.f, 0, 2.f);
		DrawDebugLine(GetWorld(), GetActorLocation(), MoveGoal, FColor::Yellow, false, -1.f, 0, 1.f);
	}
	if (Target)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), Target->GetPelvisLocation(), FColor::Green, false, -1.f, 0, 1.f);
	}
	if (Blackboard && State == EDrunkardState::Exiting)
	{
		if (const AActor* ExitDoor = Cast<AActor>(Blackboard->GetValueAsObject(ADrunkardAIController::BBKeyExitDoor)))
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), ExitDoor->GetActorLocation(), FColor::Cyan, false, -1.f, 0, 1.f);
		}
	}
#endif
}

bool ADrunkardNPC::OnHitReceived_Implementation(const FHitData& HitData)
{
	if (!HasAuthority() || !CanReceiveHit()) return false;

	// 타겟의 공격만 유효. 공격자 미상(기믹류)의 히트는 허용
	ADefaultTromboneCharacter* TargetCharacter = StateComponent ? StateComponent->GetTarget() : nullptr;
	if (HitData.HitInstigatorActor && HitData.HitInstigatorActor != TargetCharacter)
	{
		return false;
	}

	// DrunkardNPC only gets knockback
	FHitData DrunkardHitData = HitData;
	DrunkardHitData.HitReaction = EHitReactionType::KnockbackOnly;

	const bool bApplied = Super::OnHitReceived_Implementation(DrunkardHitData);

	// 타겟 공격 적중 → 타겟 변경
	if (bApplied && StateComponent)
	{
		StateComponent->RequestTargetChange();
	}

	return bApplied;
}

void ADrunkardNPC::HandleStunStateChanged(const bool bIsStunned)
{
	UCharacterMovementComponent* Move = GetCharacterMovement();

	if (bIsStunned)
	{
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->StopMovement();
		}
		if (Move)
		{
			Move->MaxWalkSpeed = 0.f;
		}
	}
	else if (Move)
	{
		Move->MaxWalkSpeed = DrunkardData ? DrunkardData->WalkSpeed : 250.f;
	}
}

void ADrunkardNPC::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, const bool bSelfMoved,
	const FVector HitLocation, const FVector HitNormal, const FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// 포획 판정은 캡슐 접촉 기준. 상체 물리 바디 등 다른 컴포넌트의 충돌은 제외
	if (MyComp == GetCapsuleComponent() && StateComponent)
	{
		StateComponent->HandleCaptureContact(Other);
	}
}

void ADrunkardNPC::SetOwningSpawner(ADrunkardSpawner* InSpawner)
{
	OwningSpawner = InSpawner;
}

ADrunkardSpawner* ADrunkardNPC::GetOwningSpawner() const
{
	return OwningSpawner.Get();
}

bool ADrunkardNPC::CanReceiveHit() const
{
	if (StateComponent && (StateComponent->GetState() == EDrunkardState::Exiting
						|| StateComponent->GetState() == EDrunkardState::Entering))
	{
		return false;
	}

	return Super::CanReceiveHit();
}
