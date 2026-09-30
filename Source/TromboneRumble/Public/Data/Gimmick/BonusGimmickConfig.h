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
UCLASS(Abstract, meta = (GroupSettingsByCategory))
class TROMBONERUMBLE_API UBonusGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void ValidateConfig(FDataValidationContext& Context) const override;
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface
#endif

	//~ No category, so it stays outside the groups of the gimmick settings panel
	/** 떨어뜨릴 블루프린트 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "떨어뜨릴 클래스"))
	TSubclassOf<ABonusDrop> DropClass;

	/** 한 번 떨어뜨린 뒤 다음 번까지의 간격. 0이면 떨어뜨리지 않는다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "생성 간격", ClampMin = "0.0", Units = "s"))
	float SpawnInterval = 10.f;

	/** 한 번에 떨어뜨릴 최소 개수 */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "최소 개수", ClampMin = "1"))
	int32 MinDropCount = 1;

	/** 한 번에 떨어뜨릴 최대 개수. 빈 지점이 모자라면 있는 만큼만 떨어진다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "최대 개수", ClampMin = "1"))
	int32 MaxDropCount = 1;

	/** 착지한 뒤 사라지기까지의 시간. 0이면 누가 주울 때까지 남는다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "유지 시간", ClampMin = "0.0", Units = "s"))
	float Lifetime = 0.f;

	/** 하나를 주웠을 때 얻는 점수 */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "획득 점수", ClampMin = "0"))
	int32 BonusScore = 300;

	/** 착지 지점에 예고를 띄운 뒤 떨어뜨리기까지의 시간. 0이면 예고 없이 바로 떨어진다 (초) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "예고 시간", ClampMin = "0.0", Units = "s"))
	float WarningDuration = 0.f;

	/** 예고 동안 착지 지점 바닥에 띄울 표시. 블루프린트의 Replicates는 꺼 둘 것 */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "예고 표시 클래스"))
	TSubclassOf<AActor> WarningMarkerClass;
};
