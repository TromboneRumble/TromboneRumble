// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Data/Gimmick/BlackHoleGimmickConfig.h"
#include "Data/Gimmick/BonusGimmickConfig.h"
#include "Data/Gimmick/DrunkardGimmickConfig.h"
#include "Data/Gimmick/GarbageGimmickConfig.h"

#if WITH_EDITOR
#include "Actors/Gimmick/Bonus/BonusDrop.h"
#include "Actors/Gimmick/Garbage/GarbageBase.h"
#include "Misc/DataValidation.h"

void UGarbageGimmickConfig::ValidateConfig(FDataValidationContext& Context) const
{
	if (GarbageClasses.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("관중 투척: 투척물 종류가 비어 있어 아무것도 던지지 않습니다")));
	}

	// The spawner picks a slot at random, so an empty slot makes some throws do nothing
	if (GarbageClasses.Contains(nullptr))
	{
		Context.AddError(FText::FromString(TEXT("관중 투척: 투척물 종류에 빈 칸이 있습니다")));
	}
}

void UDrunkardGimmickConfig::ValidateConfig(FDataValidationContext& Context) const
{
	// The gimmick still runs without it, so this is a warning and not an error
	if (!DiveMontage)
	{
		Context.AddWarning(FText::FromString(TEXT("취객: 다이브 몽타주가 비어 있어 포획 후 몸을 날리는 연출이 나오지 않습니다")));
	}
}

void UBonusGimmickConfig::ValidateConfig(FDataValidationContext& Context) const
{
	// The present and the docking port share this function, so the message takes the name from the config class
	const FText Name = GetClass()->GetDisplayNameText();

	if (!DropClass)
	{
		Context.AddError(FText::Format(FText::FromString(TEXT("{0}: 떨어뜨릴 클래스가 비어 있어 아무것도 떨어지지 않습니다")), Name));
	}

	// The spawner still works and always drops the minimum, so this is a warning and not an error
	if (MinDropCount > MaxDropCount)
	{
		Context.AddWarning(FText::Format(FText::FromString(TEXT("{0}: 최소 개수가 최대 개수보다 커서 항상 최소 개수만큼 떨어집니다")), Name));
	}

	if (WarningDuration > 0.f && !WarningMarkerClass)
	{
		Context.AddWarning(FText::Format(FText::FromString(TEXT("{0}: 예고 표시 클래스가 비어 있어 예고 시간 동안 아무것도 보이지 않습니다")), Name));
	}
}

void UBlackHoleGimmickConfig::ValidateConfig(FDataValidationContext& Context) const
{
	if (InfluenceRadiusEnd < InfluenceRadiusStart)
	{
		Context.AddError(FText::FromString(TEXT("블랙홀: 영향 반경 최대가 시작보다 작아 반경이 줄어듭니다")));
	}

	// A capture radius as wide as the influence radius leaves nothing to orbit
	if (InnerRadiusStart >= InfluenceRadiusStart)
	{
		Context.AddError(FText::FromString(TEXT("블랙홀: 포획 반경 시작이 영향 반경 시작보다 커서 공전 구간이 없습니다")));
	}

	if (HasEmptyCurve())
	{
		Context.AddError(FText::FromString(TEXT("블랙홀: 거리별 끌림/공전 커브에 키가 없어 아무것도 끌려가지 않습니다")));
	}

	// The gimmick still runs, but nobody can walk out of it
	if (EvalPull(1.f) >= 1.f)
	{
		Context.AddWarning(FText::FromString(TEXT("블랙홀: 가장자리 끌림 비율이 1 이상이라 걸어서 탈출할 수 없습니다")));
	}

	if (BurstSpeed <= 0.f)
	{
		Context.AddWarning(FText::FromString(TEXT("블랙홀: 방출 속도가 0이라 붕괴해도 튕기지 않습니다")));
	}

	// cm/s 이므로 5000이면 초속 50m다. 실내 맵에서는 벽을 넘어 낙사한다
	if (BurstSpeed > 5000.f)
	{
		Context.AddWarning(FText::FromString(TEXT("블랙홀: 방출 속도가 너무 커서 맵 밖으로 날아갑니다 (단위는 cm/s)")));
	}
}
#endif
