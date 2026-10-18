// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "DrunkardGimmickConfig.generated.h"

/** Drunkard values that differ between the normal part and the fever part of the song. */
USTRUCT(BlueprintType)
struct FDrunkardRule
{
	GENERATED_BODY()

	/** 이동 속도. 악기를 든 플레이어의 달리기(480)보다 느려야 한다 (cm/s) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "이동 속도", ClampMin = "0.0", Units = "cm/s"))
	float WalkSpeed = 400.f;

	/** 악기를 든 타겟을 포획(래그돌)할 때의 수평 넉백 (cm/s) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "포획 넉백 세기 (악기 보유)", ClampMin = "0.0", Units = "cm/s"))
	float CaptureKnockbackForce = 200.f;

	/** 악기를 든 타겟을 포획(래그돌)할 때의 위쪽 넉백 (cm/s) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "포획 상향 넉백 (악기 보유)", ClampMin = "0.0", Units = "cm/s"))
	float CaptureKnockbackUpForce = 100.f;

	/** 악기가 없는 타겟을 포획(스턴)할 때의 수평 넉백 (cm/s) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "포획 넉백 세기 (악기 없음)", ClampMin = "0.0", Units = "cm/s"))
	float CaptureKnockbackForceNoInstrument = 500.f;

	/** 악기가 없는 타겟을 포획(스턴)할 때의 위쪽 넉백 (cm/s) */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "포획 상향 넉백 (악기 없음)", ClampMin = "0.0", Units = "cm/s"))
	float CaptureKnockbackUpForceNoInstrument = 300.f;
};

/**
 * Settings of the drunkard gimmick. The spawner, the NPC, its state component and its behavior tree nodes all read this one object.
 * It replaced DA_Drunkard, and the defaults are the values that asset held.
 * Stun and invincibility times are shared by every character, so they stay in UCharacterDataAsset.
 * The dive montage, the upper body physics and the X-Ray silhouette depend on the mesh, so they stay on ADrunkardNPC.
 *
 * @see ADrunkardSpawner
 * @see ADrunkardNPC
 */
UCLASS(meta = (DisplayName = "취객"))
class TROMBONERUMBLE_API UDrunkardGimmickConfig : public UEventGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UDrunkardGimmickConfig()
	{
		GimmickType = EGimmickType::Drunkard;

		// The drunkard breaks in without a warning. Its entrance plays the part of one
		bHasWarning = false;
	}

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual float BuildEventTimeline(FGimmickTimelineBuilder& Builder, float Start) const override;
	//~ End UGimmickConfig Interface
#endif

	//~ Begin UEventGimmickConfig Interface
	virtual void GetEventPhases(const UWorld* World, TArray<FGimmickPhase>& OutPhases) const override;
	//~ End UEventGimmickConfig Interface

	/** @return The rule for the current part of the song. The fever rule counts only while bUseFeverRule is on. */
	const FDrunkardRule& GetRule(const bool bIsFeverTime) const { return bIsFeverTime && bUseFeverRule ? Fever : Normal; }

	/** @return Walk speed for the current part of the song. */
	float GetWalkSpeed(const bool bIsFeverTime) const { return GetRule(bIsFeverTime).WalkSpeed; }

	/**
	 * Get the knockback of a capture.
	 *
	 * @param bIsFeverTime Whether fever time has started.
	 * @param bHasInstrument Whether the captured target holds an instrument.
	 * @param OutForce Horizontal knockback (cm/s).
	 * @param OutUpForce Upward knockback (cm/s).
	 */
	void GetCaptureKnockback(const bool bIsFeverTime, const bool bHasInstrument, float& OutForce, float& OutUpForce) const
	{
		const FDrunkardRule& Rule = GetRule(bIsFeverTime);
		OutForce = bHasInstrument ? Rule.CaptureKnockbackForce : Rule.CaptureKnockbackForceNoInstrument;
		OutUpForce = bHasInstrument ? Rule.CaptureKnockbackUpForce : Rule.CaptureKnockbackUpForceNoInstrument;
	}

	/** 평소에 쓰는 값. 피버 타임 시 강화를 끄면 피버 타임에도 이 값을 쓴다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|규칙", meta = (DisplayName = "일반"))
	FDrunkardRule Normal;

	/** 켜면 피버 타임부터 피버 값을 쓰고, 이미 나와 있는 취객도 바로 바뀐다. 끄면 곡 내내 일반 값을 쓴다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|규칙", meta = (DisplayName = "피버 타임 시 강화"))
	bool bUseFeverRule = true;

	/** 피버 타임 시 강화를 켰을 때 피버 타임부터 쓰는 값 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|규칙", meta = (DisplayName = "피버", EditCondition = "bUseFeverRule"))
	FDrunkardRule Fever;

	/** 문을 통과한 뒤 추격을 시작하기 전까지 멈춰 있는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|시간", meta = (DisplayName = "등장 후 정지 시간", ClampMin = "0.0", Units = "s"))
	float EnterPauseDuration = 1.f;

	/** 생성 지점에서 정지 지점까지 문을 뚫고 들어오는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|시간", meta = (DisplayName = "문 통과 시간", ClampMin = "0.0", Units = "s"))
	float DoorPassDuration = 0.5f;

	/** 취객이 문을 부술 때 문 조각이 튀어 나가는 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "문 파편 초속", ClampMin = "0.0", Units = "cm/s"))
	float DoorBreakStrength = 600.f;

	/** 플레이어를 쫓는 시간. 다 쓰면 들어온 문으로 퇴장한다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|시간", meta = (DisplayName = "추격 시간", ClampMin = "0.0", Units = "s"))
	float ChaseDuration = 30.f;

	/** 이 시간 안에 퇴장 지점에 닿지 못하면(경로 막힘 등) 강제로 사라진다. 끼인 취객이 다음 등장을 막지 않게 한다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|시간", meta = (DisplayName = "퇴장 제한 시간", ClampMin = "1.0", Units = "s"))
	float ExitTimeout = 10.f;

	/** 목표 속도까지 붙는 가속도 (cm/s²) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|이동", meta = (DisplayName = "최대 가속도", ClampMin = "0.0", Units = "cm/s^2"))
	float MaxAcceleration = 800.f;

	/** 타겟 플레이어 화면에서 취객 머리 위 아이콘이 얼굴에서 공격 표시로 바뀌는 거리 (cm) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "공격 표시 전환 거리", ClampMin = "0.0", Units = "cm"))
	float AttackableIndicatorRange = 500.f;

	/** 경로가 좌우로 흔들리는 폭 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|이동", meta = (DisplayName = "위빙 진폭", ClampMin = "0.0", Units = "cm"))
	float WeaveAmplitude = 2000.f;

	/** 흔들림이 반복되는 빠르기. 낮을수록 한쪽으로 길게 쏠렸다가 돌아온다 (rad/s) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|이동", meta = (DisplayName = "위빙 주파수", ClampMin = "0.0", ForceUnits = "rad/s"))
	float WeaveFrequency = 5.f;

	/** 흔들림 폭을 무작위로 바꾸는 비율. 0.3이면 ±30%로, 기계적인 사인파처럼 보이지 않게 한다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|이동", meta = (DisplayName = "진폭 변주 비율", ClampMin = "0.0", ClampMax = "1.0"))
	float WeaveAmplitudeNoise = 0.6f;

	/** 이동 속도를 주기적으로 바꾸는 비율. 0.15면 ±15%로, 휘청였다 회복하는 느낌을 준다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|이동", meta = (DisplayName = "속도 변주 비율", ClampMin = "0.0", ClampMax = "1.0"))
	float SpeedVariance = 0.6f;

	/** 이동 목표에 닿았다고 보는 반경. 커지면 목표 근처에서 흔들림이 사라진다 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|이동", meta = (DisplayName = "도달 판정 반경", ClampMin = "0.0", Units = "cm"))
	float MoveAcceptanceRadius = 50.f;

	/** 순위별로 타겟이 될 가중치. 첫 칸이 1위이고, 순위가 칸 수를 넘으면 마지막 값을 쓴다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|규칙", meta = (DisplayName = "순위별 타겟 가중치"))
	TArray<float> TargetRankWeights = { 4.f, 3.f, 2.f, 1.f };

	/** 정점에서 래그돌로 바뀔 때 앞으로 엎어지는 회전 속도. 부호를 뒤집으면 반대로 돈다 (rad/s) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "다이브 회전 속도", ForceUnits = "rad/s"))
	float DiveSpinSpeed = 3.f;

	/** 다이브가 이 시간 안에 끝나지 않으면 강제로 퇴장한다. 몽타주나 노티파이가 빠져 기믹이 멈추는 것을 막는다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|시간", meta = (DisplayName = "다이브 제한 시간", ClampMin = "1.0", Units = "s"))
	float DiveTimeout = 10.f;
};
