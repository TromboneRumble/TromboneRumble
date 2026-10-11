// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/BonusGimmickConfig.h"
#include "DockingPortGimmickConfig.generated.h"

/**
 * Settings of the docking port supply of the space station, dropped by ABonusSpawner.
 *
 * @see ABonusSpawner
 * @see ADockingPort
 */
UCLASS(meta = (DisplayName = "도킹 포트 보급", GroupSettingsByCategory))
class TROMBONERUMBLE_API UDockingPortGimmickConfig : public UBonusGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UDockingPortGimmickConfig()
	{
		GimmickType = EGimmickType::DockingPort;
		MaxDropCount = 2;
		WarningDuration = 0.f;
		Lifetime = 10.f;
	}

	/** 감속하기 전까지 떨어지는 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "낙하 속도", ClampMin = "1.0", Units = "cm/s"))
	float FallSpeed = 1000.f;

	/** 착지 지점보다 이만큼 위에서부터 감속해 멈춘다 (cm) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "감속 시작 높이", ClampMin = "0.0", Units = "cm"))
	float BrakeHeight = 400.f;
};
