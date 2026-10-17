// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "BonusGimmickConfig.generated.h"

class ABonusDrop;

/**
 * Settings of ABonusSpawner, shared by every gimmick that drops bonus score from the sky.
 * Each gimmick has a small subclass that only sets its type and its own defaults.
 *
 * @see ABonusSpawner
 * @see ABonusDrop
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API UBonusGimmickConfig : public USpawnerGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. The first drop waits one interval, as the bonus gimmicks always did. */
	UBonusGimmickConfig()
	{
		FirstSpawnDelay = 10.f;
		Normal.Interval.Min = 10.f;
		Normal.Interval.Max = 10.f;
	}

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void ValidateConfig(FDataValidationContext& Context) const override;
	//~ End UGimmickConfig Interface

	//~ Begin USpawnerGimmickConfig Interface
	virtual float BuildSpawnTimeline(FGimmickTimelineBuilder& Builder, float SpawnTime) const override;
	//~ End USpawnerGimmickConfig Interface
#endif

	//~ No category, so it stays outside the groups of the gimmick settings panel
	/** 떨어뜨릴 블루프린트 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "떨어뜨릴 클래스"))
	TSubclassOf<ABonusDrop> DropClass;

	/** 착지한 뒤 사라지기까지의 시간. 0이면 누가 주울 때까지 남는다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|물체", meta = (DisplayName = "유지 시간", ClampMin = "0.0", Units = "s"))
	float Lifetime = 0.f;

	/** 하나를 주웠을 때 얻는 점수 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|물체", meta = (DisplayName = "획득 점수", ClampMin = "0"))
	int32 BonusScore = 300;

	/** 착지 지점에 예고를 띄운 뒤 떨어뜨리기까지의 시간. 0이면 예고 없이 바로 떨어진다 (초) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "예고 시간", ClampMin = "0.0", Units = "s"))
	float WarningDuration = 0.f;

	/** 예고 동안 착지 지점 바닥에 띄울 표시. 블루프린트의 Replicates는 꺼 둘 것 */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "예고 표시 클래스"))
	TSubclassOf<AActor> WarningMarkerClass;
};
