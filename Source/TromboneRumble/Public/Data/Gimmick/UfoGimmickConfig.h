// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "UfoGimmickConfig.generated.h"

/**
 * Settings of AUfoGimmick.
 *
 * @see AUfoGimmick
 */
UCLASS(meta = (DisplayName = "미니 UFO", GroupSettingsByCategory))
class TROMBONERUMBLE_API UUfoGimmickConfig : public UEventGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UUfoGimmickConfig()
	{
		GimmickType = EGimmickType::Ufo;
		WarningDuration = 2.f;
	}

	/** @return Height the UFO flies at, so its beam ends right on the floor. */
	float GetFlightZ() const { return FloorZ + BeamLength; }

	/** @return HangDistance, kept inside the beam so a player never hangs below the floor. */
	float GetHangDistance() const { return FMath::Min(HangDistance, BeamLength); }

	/** @return Seconds from the spawn until the UFO starts along its line: the warp in, then the beam spreading down. */
	float GetIntroDuration() const { return WarpDuration + BeamDeployDuration; }

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual float BuildEventTimeline(FGimmickTimelineBuilder& Builder, float Start) const override;
	//~ End UGimmickConfig Interface
#endif

	/** UFO가 라인을 따라 이동하는 속도. 느릴수록 광선이 오래 켜져 붙잡힌 플레이어도 오래 매달린다 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "이동 속도", ClampMin = "1.0", Units = "cm/s"))
	float MoveSpeed = 700.f;

	/** 바닥에서 UFO까지의 높이. 광선은 항상 여기서 바닥까지 닿는다. 벽 높이가 바뀌면 이 값을 바꾼다 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "비행 높이", ClampMin = "10.0", Units = "cm"))
	float BeamLength = 600.f;

	/** 인게임에서 가장 낮은 바닥의 Z 값. 광선 끝이 여기에 닿는다. 레벨 바닥 높이가 바뀔 때만 수정 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "바닥 높이", Units = "cm"))
	float FloorZ = 0.f;

	/** 붙잡힌 플레이어가 매달리는 위치. UFO에서 이만큼 아래이며 광선 길이보다 길어지지 않는다 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "매달리는 거리", ClampMin = "0.0", Units = "cm"))
	float HangDistance = 250.f;

	/** 붙잡힌 플레이어가 바닥에서 매달리는 위치까지 올라가는 최대 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "끌어올리는 최대 속도", ClampMin = "10.0", Units = "cm/s"))
	float MaxLiftSpeed = 800.f;

	/** 매달린 뒤 매달리는 위치에 붙어 있으려는 세기. 높을수록 UFO를 바짝 따라가지만 더 흔들린다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "끌어올리는 세기", ClampMin = "0.5", ClampMax = "30.0"))
	float LiftStrength = 10.f;

	/** 붙잡힌 플레이어가 매달려 있는 동안 도는 속도. 0이면 돌지 않는다 (도/초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "붙잡힌 플레이어 회전 속도", ClampMin = "0.0", Units = "deg/s"))
	float LiftedSpinSpeed = 30.f;

	/** 광선이 다 뻗거나 다 접히는 시간. 다 뻗은 뒤부터 잡고, 라인 끝에서 접히기 시작하면 붙잡힌 플레이어가 풀려난다 (초) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "광선 펼침 시간", ClampMin = "0.0", Units = "s"))
	float BeamDeployDuration = 0.3f;

	/** 워프로 날아와 라인 시작점에 멈추는 시간. 떠날 때도 같은 시간 동안 날아가 사라진다 (초) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "워프 시간", ClampMin = "0.05", Units = "s"))
	float WarpDuration = 1.f;

	/** 라인 뒤쪽 이만큼 멀리서 날아오고, 라인 앞쪽으로 이만큼 날아가 사라진다 (cm) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "워프 거리", ClampMin = "0.0", Units = "cm"))
	float WarpDistance = 3000.f;
};
