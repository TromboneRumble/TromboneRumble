// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "BlackHoleGimmickConfig.generated.h"

/**
 * Settings of ABlackHoleGimmick.
 *
 * The two curves are the whole shape of the orbit, so no other number decides the path.
 * X is the distance divided by the influence radius, 0 at the center and 1 at the edge.
 * Y is a ratio of a reference speed: the walk speed for a player, PhysicsReferenceSpeed for a prop.
 * A distance where the pull curve reads 0 becomes a ring the objects circle on.
 *
 * @see ABlackHoleGimmick
 */
UCLASS(meta = (DisplayName = "블랙홀"))
class TROMBONERUMBLE_API UBlackHoleGimmickConfig : public UEventGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UBlackHoleGimmickConfig()
	{
		GimmickType = EGimmickType::BlackHole;

		// Strong and fast near the center, weak and wide at the edge
		PullCurve.EditorCurveData.AddKey(0.f, 0.8f);
		PullCurve.EditorCurveData.AddKey(1.f, 0.25f);
		OrbitCurve.EditorCurveData.AddKey(0.f, 0.6f);
		OrbitCurve.EditorCurveData.AddKey(1.f, 0.2f);
	}

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void ValidateConfig(FDataValidationContext& Context) const override;
	//~ End UGimmickConfig Interface
#endif

	//~ Begin UEventGimmickConfig Interface
	virtual void GetEventPhases(const UWorld* World, TArray<FGimmickPhase>& OutPhases) const override;
	//~ End UEventGimmickConfig Interface

	/** @return Pull ratio at DistanceAlpha, 0 at the center and 1 at the edge. */
	float EvalPull(const float DistanceAlpha) const { return PullCurve.GetRichCurveConst()->Eval(DistanceAlpha); }

	/** @return Orbit ratio at DistanceAlpha. */
	float EvalOrbit(const float DistanceAlpha) const { return OrbitCurve.GetRichCurveConst()->Eval(DistanceAlpha); }

	/** @return Does either curve have no key at all, which would mean no pull. */
	bool HasEmptyCurve() const { return PullCurve.GetRichCurveConst()->IsEmpty() || OrbitCurve.GetRichCurveConst()->IsEmpty(); }

	/** 블랙홀이 시작 반경에서 최대 반경까지 자라는 시간. 이 동안 계속 끌어당긴다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|시간", meta = (DisplayName = "성장 시간", ClampMin = "0.0", Units = "s"))
	float GrowDuration = 12.f;

	/** 다 자란 뒤 터지기까지의 시간. 이 동안에도 계속 끌어당긴다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|시간", meta = (DisplayName = "붕괴 시간", ClampMin = "0.0", Units = "s"))
	float CollapseDuration = 0.5f;

	/** 블랙홀이 처음 열릴 때 끌어당기는 범위. 성장 시간 동안 최대까지 커진다 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|반경", meta = (DisplayName = "영향 반경 시작", ClampMin = "1.0", Units = "cm"))
	float InfluenceRadiusStart = 600.f;

	/** 다 자랐을 때 끌어당기는 범위 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|반경", meta = (DisplayName = "영향 반경 최대", ClampMin = "1.0", Units = "cm"))
	float InfluenceRadiusEnd = 1800.f;

	/** 이 안에 닿으면 플레이어는 래그돌이 되고 소품은 붙잡힌다. 붙잡힌 것만 붕괴 때 튕겨 나간다 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|반경", meta = (DisplayName = "포획 반경 시작", ClampMin = "1.0", Units = "cm"))
	float CaptureRadiusStart = 100.f;

	/** 다 자랐을 때의 포획 반경 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|반경", meta = (DisplayName = "포획 반경 최대", ClampMin = "1.0", Units = "cm"))
	float CaptureRadiusEnd = 300.f;

	/** 중심으로 끌려가는 속도. X는 중심 0부터 가장자리 1까지의 거리 비율, Y는 기준 속도에 대한 비율이며 가장자리 값이 1 이상이면 걸어서 빠져나갈 수 없다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|끌림", meta = (DisplayName = "접근 끌림 비율", XAxisName = "거리 비율", YAxisName = "끌림 비율"))
	FRuntimeFloatCurve PullCurve;

	/** 끌려가는 경로를 옆으로 휘게 하는 속도. 축은 접근 끌림 비율과 같고, 붙잡힌 것은 이 대신 고리를 돈다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|끌림", meta = (DisplayName = "접근 선회 비율", XAxisName = "거리 비율", YAxisName = "공전 비율"))
	FRuntimeFloatCurve OrbitCurve;

	/** 켜면 위에서 볼 때 시계 방향으로 돈다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|끌림", meta = (DisplayName = "시계 방향 공전"))
	bool bOrbitClockwise = true;

	/** 소품과 래그돌에 커브 비율을 곱할 기준 속도. 걷는 플레이어는 자기 걷기 속도를 쓴다 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "고급", meta = (DisplayName = "소품/래그돌 기준 속도", ClampMin = "0.0", Units = "cm/s"))
	float PhysicsReferenceSpeed = 300.f;

	/** 커브가 원하는 속도로 몸을 맞추는 세기. 높을수록 궤도에 바로 붙고 낮을수록 미끄러진다 */
	UPROPERTY(EditAnywhere, Category = "고급", meta = (DisplayName = "조향 강도", ClampMin = "0.1"))
	float SteerGain = 5.f;

	/** 조향의 상한. 원하는 속도와 차이가 큰 몸이 갑자기 튕기지 않게 한다 (cm/s²) */
	UPROPERTY(EditAnywhere, Category = "고급", meta = (DisplayName = "최대 조향 가속도", ClampMin = "1.0", Units = "cm/s^2"))
	float MaxSteerAccel = 2000.f;

	/** 소품과 래그돌을 위아래로 끌어당기는 비율. 0이면 자기 높이를 유지한다 */
	UPROPERTY(EditAnywhere, Category = "고급", meta = (DisplayName = "수직 끌림 배율", ClampMin = "0.0", ClampMax = "1.0"))
	float VerticalPullScale = 1.f;

	/** 붙잡힌 것이 도는 고리의 반경. 포획 반경에 대한 비율이며 블랙홀과 함께 커진다 */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "고리 반경 비율", ClampMin = "0.1", ClampMax = "1.5"))
	float RingRadiusRatio = 0.75f;

	/** 물체마다 고리 반경을 무작위로 벌리는 폭. 클수록 고리가 띠처럼 넓어진다 (cm) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "고리 반경 흔들림", ClampMin = "0.0", Units = "cm"))
	float RingRadiusJitter = 60.f;

	/** 물체마다 블랙홀 중심 높이에서 무작위로 벗어나는 높이 (cm) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "고리 높이 흔들림", ClampMin = "0.0", Units = "cm"))
	float RingHeightJitter = 40.f;

	/** 붙잡힌 것이 고리를 도는 속도 (도/초) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "고리 각속도", ClampMin = "0.0", Units = "deg/s"))
	float RingAngularSpeed = 120.f;

	/** 물체마다 도는 속도를 무작위로 바꾸는 폭 (도/초) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "고리 각속도 흔들림", ClampMin = "0.0", Units = "deg/s"))
	float RingAngularSpeedJitter = 30.f;

	/** 붙잡힌 것을 고리 반경과 높이에 붙잡아 두는 세기. 높을수록 바로 붙고 낮을수록 천천히 다가간다 */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "고리 수렴 강도", ClampMin = "0.1"))
	float RingSpring = 2.f;

	/** 붕괴할 때 붙잡힌 것들이 튕겨 나가는 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|방출", meta = (DisplayName = "방출 속도", ClampMin = "0.0", Units = "cm/s"))
	float BurstSpeed = 1500.f;

	/** 튕겨 나가는 힘 중 위로 향하는 비율. 0이면 수평으로만 날아간다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|방출", meta = (DisplayName = "방출 상승 비율", ClampMin = "0.0", ClampMax = "1.0"))
	float BurstUpRatio = 0.35f;
};
