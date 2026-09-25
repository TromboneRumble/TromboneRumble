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
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	virtual void ValidateConfig(FDataValidationContext& Context) const override;
	//~ End UGimmickConfig Interface
#endif

	/** @return Pull ratio at DistanceAlpha, 0 at the center and 1 at the edge. */
	float EvalPull(const float DistanceAlpha) const { return PullCurve.GetRichCurveConst()->Eval(DistanceAlpha); }

	/** @return Orbit ratio at DistanceAlpha. */
	float EvalOrbit(const float DistanceAlpha) const { return OrbitCurve.GetRichCurveConst()->Eval(DistanceAlpha); }

	/** @return Does either curve have no key at all, which would mean no pull. */
	bool HasEmptyCurve() const { return PullCurve.GetRichCurveConst()->IsEmpty() || OrbitCurve.GetRichCurveConst()->IsEmpty(); }

	/** Seconds the hole takes to grow from its start radius to the full one. */
	UPROPERTY(EditAnywhere, Category = "Timing", meta = (DisplayName = "성장 시간", ClampMin = "0.5", Units = "s"))
	float ActiveDuration = 12.f;

	/** Seconds between the hole reaching full size and the burst. The pull keeps running. */
	UPROPERTY(EditAnywhere, Category = "Timing", meta = (DisplayName = "붕괴 시간", ClampMin = "0.05", Units = "s"))
	float CollapseDuration = 0.5f;

	/** Radius the pull reaches when the hole appears. */
	UPROPERTY(EditAnywhere, Category = "Radius", meta = (DisplayName = "영향 반경 시작", ClampMin = "1.0", Units = "cm"))
	float InfluenceRadiusStart = 600.f;

	/** Radius the pull reaches when the hole is fully grown. */
	UPROPERTY(EditAnywhere, Category = "Radius", meta = (DisplayName = "영향 반경 최대", ClampMin = "1.0", Units = "cm"))
	float InfluenceRadiusEnd = 1800.f;

	/** Touching this radius ragdolls a player and captures a prop. Only captured things are thrown out. */
	UPROPERTY(EditAnywhere, Category = "Radius", meta = (DisplayName = "포획 반경 시작", ClampMin = "1.0", Units = "cm"))
	float InnerRadiusStart = 100.f;

	UPROPERTY(EditAnywhere, Category = "Radius", meta = (DisplayName = "포획 반경 최대", ClampMin = "1.0", Units = "cm"))
	float InnerRadiusEnd = 300.f;

	/** Speed toward the center while an object is still on its way in. 1 at the edge means nobody can walk out. */
	UPROPERTY(EditAnywhere, Category = "Orbit", meta = (DisplayName = "접근 끌림 비율", XAxisName = "거리 비율", YAxisName = "끌림 비율"))
	FRuntimeFloatCurve PullCurve;

	/** Sideways speed that bends the approach path. Same axes as the pull curve. A captured object leaves this behind and uses the ring instead. */
	UPROPERTY(EditAnywhere, Category = "Orbit", meta = (DisplayName = "접근 선회 비율", XAxisName = "거리 비율", YAxisName = "공전 비율"))
	FRuntimeFloatCurve OrbitCurve;

	/** Which way everything circles, seen from above. */
	UPROPERTY(EditAnywhere, Category = "Orbit", meta = (DisplayName = "시계 방향 공전"))
	bool bOrbitClockwise = true;

	/** Speed the curves scale for props and ragdolls, in cm/s. A walking player uses their own walk speed instead. */
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (DisplayName = "소품/래그돌 기준 속도 (cm/s)", ClampMin = "0.0"))
	float PhysicsReferenceSpeed = 300.f;

	/** How hard a body is steered toward the speed the curves ask for. Higher snaps onto the orbit, lower drifts. */
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (DisplayName = "조향 강도", ClampMin = "0.1"))
	float SteerGain = 5.f;

	/** Ceiling of the steering, so a body far from the wanted speed is not kicked. */
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (DisplayName = "최대 조향 가속도", ClampMin = "1.0"))
	float MaxSteerAccel = 2000.f;

	/** Vertical part of the pull on props and ragdolls. 0 keeps them at their own height. */
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (DisplayName = "수직 끌림 배율", ClampMin = "0.0", ClampMax = "1.0"))
	float VerticalPullScale = 1.f;

	/**
	 * Radius the ring sits at, as a ratio of the capture radius. The ring grows with the hole.
	 * A captured object settles here and circles instead of winding into the center.
	 */
	UPROPERTY(EditAnywhere, Category = "Ring", meta = (DisplayName = "고리 반경 비율", ClampMin = "0.1", ClampMax = "1.5"))
	float RingRadiusRatio = 0.75f;

	/** Random spread of the ring radius per object, in cm. Spread turns the ring from a wire into a band. */
	UPROPERTY(EditAnywhere, Category = "Ring", meta = (DisplayName = "고리 반경 흔들림", ClampMin = "0.0", Units = "cm"))
	float RingRadiusJitter = 60.f;

	/** Random height around the center of the hole, in cm. */
	UPROPERTY(EditAnywhere, Category = "Ring", meta = (DisplayName = "고리 높이 흔들림", ClampMin = "0.0", Units = "cm"))
	float RingHeightJitter = 40.f;

	/** How fast a captured object goes around, in degrees per second. */
	UPROPERTY(EditAnywhere, Category = "Ring", meta = (DisplayName = "고리 각속도 (도/초)", ClampMin = "0.0"))
	float RingAngularSpeed = 120.f;

	/** Random spread of the angular speed per object, in degrees per second. */
	UPROPERTY(EditAnywhere, Category = "Ring", meta = (DisplayName = "고리 각속도 흔들림 (도/초)", ClampMin = "0.0"))
	float RingAngularSpeedJitter = 30.f;

	/** How hard a captured object is held on its ring radius and height. Higher snaps onto the ring, lower drifts toward it. */
	UPROPERTY(EditAnywhere, Category = "Ring", meta = (DisplayName = "고리 수렴 강도", ClampMin = "0.1"))
	float RingSpring = 2.f;

	/** Speed the captured things leave with when the hole collapses, in cm/s. */
	UPROPERTY(EditAnywhere, Category = "Burst", meta = (DisplayName = "방출 속도 (cm/s)", ClampMin = "0.0"))
	float BurstSpeed = 1500.f;

	/** How much of the burst goes up instead of out. */
	UPROPERTY(EditAnywhere, Category = "Burst", meta = (DisplayName = "방출 상승 비율", ClampMin = "0.0", ClampMax = "1.0"))
	float BurstUpRatio = 0.35f;
};
