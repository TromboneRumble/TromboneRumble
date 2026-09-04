// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "DrunkardDataAsset.generated.h"

class UAnimMontage;

/*
 * 재즈바 취객 NPC 기믹 데이터.
 * 상태 수치(스턴/무적 시간)는 캐릭터 공통이므로 UCharacterDataAsset을 그대로 사용한다.
 */
UCLASS()
class TROMBONERUMBLE_API UDrunkardDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UDrunkardDataAsset()
	{
		// 상체 물리 시작값: 세기(Strength)를 낮출수록 흐물거림이 커진다.
		UpperBodyPhysicsProfile.bIsLocalSimulation = false;
		UpperBodyPhysicsProfile.OrientationStrength = 200.f;
		UpperBodyPhysicsProfile.AngularVelocityStrength = 20.f;
		UpperBodyPhysicsProfile.PositionStrength = 200.f;
		UpperBodyPhysicsProfile.VelocityStrength = 20.f;
	}

	/** 라운드 시작 후 최초 스폰까지 대기 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Spawn", meta = (DisplayName = "최초 스폰 시간", ClampMin = "0.0"))
	float InitialSpawnDelay = 20.f;

	/** NPC 퇴장 후 다음 스폰까지 대기 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Spawn", meta = (DisplayName = "반복 스폰 주기", ClampMin = "0.0"))
	float RespawnInterval = 20.f;

	/** 문 통과 후 추격 시작 전까지 멈춰 있는 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Spawn", meta = (DisplayName = "등장 후 정지 시간", ClampMin = "0.0"))
	float EnterDuration = 1.f;

	/** 문 뒤 스폰 거리 (cm). 문 forward 반대편에 스폰되며, 반대로 스폰되면 부호를 뒤집으면 된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Spawn", meta = (DisplayName = "문 뒤 스폰 거리"))
	float BehindDoorOffset = 150.f;

	/** 문 뒤에서 실내까지 통과 이동에 걸리는 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Spawn", meta = (DisplayName = "문 통과 시간", ClampMin = "0.05"))
	float EnterBurstDuration = 0.5f;

	/** 추격 지속시간. 소진되면 가장 가까운 문으로 퇴장한다 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Duration", meta = (DisplayName = "기믹 지속시간", ClampMin = "0.0"))
	float ChaseDuration = 10.f;

	/** 퇴장 제한 시간. 이 시간 안에 문에 도달해 소멸하지 못하면(경로 막힘 등) 강제 소멸한다.
	 *  NPC가 끼어 있으면 재스폰 루프 전체가 멈추므로 페일세이프 필수 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Duration", meta = (DisplayName = "퇴장 제한 시간", ClampMin = "1.0"))
	float ExitTimeout = 10.f;

	/** 기본 이동 속도. 악기를 장착한 플레이어의 달리기(480)보다 느려야 한다 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement", meta = (DisplayName = "이동 속도", ClampMin = "0.0"))
	float WalkSpeed = 250.f;

	/** 목표 속도에 도달하기까지의 가속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement", meta = (DisplayName = "최대 가속도", ClampMin = "0.0"))
	float MaxAcceleration = 800.f;

	/** 경로 좌우 흔들림 폭 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Weave", meta = (DisplayName = "위빙 진폭", ClampMin = "0.0"))
	float WeaveAmplitude = 300.f;

	/** 흔들림 주기 (rad/s). 낮을수록 한쪽으로 길게 쏠렸다가 돌아온다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Weave", meta = (DisplayName = "위빙 주파수", ClampMin = "0.0"))
	float WeaveFrequency = 1.5f;

	/** 진폭 랜덤 변주 비율 (0.3 = ±30%). 기계적인 사인파로 보이는 것을 방지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Weave", meta = (DisplayName = "진폭 변주 비율", ClampMin = "0.0", ClampMax = "1.0"))
	float WeaveAmplitudeNoise = 0.3f;

	/** 이동 속도 주기적 변주 비율 (0.15 = ±15%). 휘청였다 회복하는 느낌 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Weave", meta = (DisplayName = "속도 변주 비율", ClampMin = "0.0", ClampMax = "1.0"))
	float SpeedVariance = 0.15f;

	/** 이동 목표 도달 판정 반경. 커지면 목표 근처에서 위빙이 죽는다 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Weave", meta = (DisplayName = "도달 판정 반경", ClampMin = "0.0"))
	float MoveAcceptanceRadius = 50.f;

	/** 순위별 타겟 선정 가중치. [0] = 1위. 순위가 배열 길이를 넘으면 마지막 값을 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Target", meta = (DisplayName = "순위별 타겟 가중치"))
	TArray<float> TargetRankWeights = { 4.f, 3.f, 2.f, 1.f };
	
	/** 악기 보유 타겟 포획(래그돌) 시 수평 넉백 (래그돌 초기 속도, cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Capture", meta = (DisplayName = "포획 넉백 세기 (악기 보유)", ClampMin = "0.0"))
	float CaptureKnockbackForce = 300.f;

	/** 악기 보유 타겟 포획(래그돌) 시 수직(상향) 넉백 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Capture", meta = (DisplayName = "포획 상향 넉백 (악기 보유)", ClampMin = "0.0"))
	float CaptureKnockbackUpForce = 200.f;

	/** 악기 미보유 타겟 접촉 시 수평 넉백 (cm/s). 래그돌 없이 밀려나기만 한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Capture", meta = (DisplayName = "포획 넉백 세기 (악기 없음)", ClampMin = "0.0"))
	float CaptureKnockbackForceNoInstrument = 300.f;

	/** 악기 미보유 타겟 접촉 시 수직(상향) 넉백 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Capture", meta = (DisplayName = "포획 상향 넉백 (악기 없음)", ClampMin = "0.0"))
	float CaptureKnockbackUpForceNoInstrument = 200.f;

	/** 포획 성공 후 대상 자리로 몸을 날리는 다이브 몽타주. 비워두면 도약만 하고 정점 전환이 없다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Dive", meta = (DisplayName = "다이브 몽타주"))
	TObjectPtr<UAnimMontage> DiveMontage;

	/** 정점에서 래그돌로 바뀔 때 앞으로 엎어지는 회전 속도 (rad/s). 부호를 뒤집으면 반대로 돈다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Dive", meta = (DisplayName = "다이브 회전 속도"))
	float DiveSpinSpeed = 6.f;

	/** 다이브가 이 시간 안에 안 끝나면 강제 퇴장. 몽타주/노티파이 누락으로 기믹이 멈추는 것을 막는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Dive", meta = (DisplayName = "다이브 제한 시간", ClampMin = "1.0"))
	float DiveTimeout = 10.f;

	/** 상체 물리 활성화 (지정 본 이상만 시뮬레이션. 하반신은 애니메이션 유지 — 캡슐 이탈 방지) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Visual", meta = (DisplayName = "상체 물리 사용"))
	bool bEnableUpperBodyPhysics = true;

	/** 상체 물리를 적용할 최상위 본 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Visual", meta = (DisplayName = "상체 물리 시작 본", EditCondition = "bEnableUpperBodyPhysics"))
	FName UpperBodyPhysicsRootBone = FName("spine_01");

	/** 상체 물리 프로파일 (Physical Animation 세기) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Visual", meta = (DisplayName = "상체 물리 프로파일", EditCondition = "bEnableUpperBodyPhysics"))
	FPhysicalAnimationData UpperBodyPhysicsProfile;

	/** 벽 뒤 실루엣(X-Ray) 표시 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Visual", meta = (DisplayName = "X-Ray 실루엣 사용"))
	bool bEnableXRaySilhouette = true;
};
