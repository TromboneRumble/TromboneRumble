// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DrunkardStateComponent.generated.h"

class ADefaultTromboneCharacter;
class UDrunkardDataAsset;

UENUM(BlueprintType)
enum class EDrunkardState : uint8
{
	None,
	Entering,	// 문에서 등장 연출 중
	Chasing,	// 타겟 추격 중
	Exiting,	// 문으로 퇴장 중 (피격/포획 판정 비활성)
	Diving,		// 포획 성공 후 대상 자리로 몸을 날리는 연출 중 (BT 데코레이터가 열거값을 숫자로 저장하므로 끝에만 추가할 것)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDrunkardStateChanged, EDrunkardState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDrunkardTargetChanged, ADefaultTromboneCharacter*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDrunkardCaptureSucceeded, ADefaultTromboneCharacter*, Target);

/** UDrunkardStateComponent
 *
 * 취객 NPC의 게임 규칙 소유자 — 상태 전이, 타겟 선정/변경, 지속시간 타이머. Server Only.
 * BT는 "지금 어떻게 움직일까"만 담당하고, 규칙 판정은 전부 여기서 한다.
 * 상태/타겟 변경은 델리게이트로 통지되며 AIController가 구독해 블랙보드에 반영한다.
 */
UCLASS(ClassGroup = (Drunkard), meta = (BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UDrunkardStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UDrunkardStateComponent();

	/** 스폰 직후 스포너가 호출. NPC가 문을 통과하고 잠시 멈춘 뒤 추격으로 전환된다 */
	void BeginEntering();

	/** NPC가 문 통과 이동을 마쳤을 때 호출. 정지 시간 후 추격을 시작한다 */
	void HandleDoorEntranceFinished();

	/** 문을 완전히 나온 시점. 타겟을 선정하고 지속시간 계산을 시작한다 */
	void BeginChasing();

	/** 지속시간 소진 시. 이동/디스폰은 BT가 처리한다 */
	void BeginExiting();

	/** 포획 성공 시. 다이브 연출을 시작하고, 완전히 일어나면 퇴장으로 이어진다 */
	void BeginDiving();

	/** 다이브 래그돌 종료 시 NPC가 호출. 퇴장으로 전환한다 */
	void HandleDiveFinished();

	/** 타겟 재선정 (현재 타겟 제외). 타겟이 공격을 적중시켰거나 세션에서 이탈했을 때 호출.
	 *  다른 후보가 없으면 기존 타겟을 유지한다 */
	void RequestTargetChange();

	/** NPC 소멸 (자동 생성된 AIController의 지연 정리 포함). Server Only.
	 *  스포너가 OnDestroyed를 구독하고 있어 여기서부터 재스폰 주기가 시작된다 */
	void DespawnOwner();

	/** NPC 캡슐이 블로킹 접촉했을 때 호출 (폰의 NotifyHit). 포획 판정의 진입점:
	 *  - 현재 타겟과의 접촉만 유효 (비타겟은 Block만 되고 무시), 피격 경직 중에는 발동 안 함
	 *  - 악기 보유 타겟: 래그돌 + 악기 드랍(래그돌 시 자동) + 콤보 초기화(드랍 시 자동) → 다이브 연출 후 퇴장
	 *  - 악기 미보유 타겟: 넉백만 → 타겟 변경, 계속 활동
	 *  - 이미 무력화(래그돌/스턴/무적)된 타겟: 포획 대신 타겟 변경 */
	void HandleCaptureContact(AActor* OtherActor);

	EDrunkardState GetState() const { return State; }
	ADefaultTromboneCharacter* GetTarget() const { return Target.Get(); }

	/** Entering 중 추격 전환까지 남은 시간. 타이머 비활성이면 -1 */
	float GetRemainingEnterTime() const;

	/** Chasing 중 퇴장까지 남은 지속시간. 타이머 비활성이면 -1 */
	float GetRemainingChaseTime() const;

	FOnDrunkardStateChanged OnStateChanged;
	FOnDrunkardTargetChanged OnTargetChanged;

	/** Called when an instrument-holding target has been captured. Push-only contacts do not count. */
	FOnDrunkardCaptureSucceeded OnCaptureSucceeded;

private:
	
	void SetState(EDrunkardState NewState);
	void SetTarget(ADefaultTromboneCharacter* NewTarget);

	/** 순위(점수 내림차순) 가중치 랜덤으로 타겟 선정. 순위가 높을수록 확률이 높다 */
	ADefaultTromboneCharacter* PickTargetByRankWeight(const ADefaultTromboneCharacter* Exclude) const;

	/** 순위와 무관하게 균등 확률로 타겟 선정 */
	ADefaultTromboneCharacter* PickRandomTarget(const ADefaultTromboneCharacter* Exclude) const;

	const UDrunkardDataAsset* GetData() const;
	bool HasAuthority() const;

	/** 퇴장 제한 시간 초과 — 문 도달 실패(경로 막힘 등) 시 강제 소멸. 스포너의 재스폰 루프를 살리기 위한 페일세이프 */
	void HandleExitTimeout();

	EDrunkardState State = EDrunkardState::None;

	TWeakObjectPtr<ADefaultTromboneCharacter> Target;

	FTimerHandle EnterTimerHandle;
	FTimerHandle ChaseTimerHandle;
	FTimerHandle ExitTimerHandle;
	FTimerHandle DiveTimerHandle;
	
public:
	
	//~ Begin UActorComponent Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent Interface

};
