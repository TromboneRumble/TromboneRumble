// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Utilities/Defines.h"
#include "CustomizationPartRow.generated.h"

/**
 * DataTable row for customization parts.
 * RowName(Key) = asset name used in the Unreal Editor (e.g. SM_Antenna_Star, M_Character_Face2).
 * AssetPath is empty for Order 0 (default / no part attached).
 */
USTRUCT(BlueprintType)
struct FCustomizationPartRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "개발자/기획자용 식별 이름"))
	FString DevName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "장착 슬롯 종류 (Antenna / Face / Costume)"))
	ECustomizationSlotType SlotType = ECustomizationSlotType::Invalid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "파츠 에셋 경로. 비어 있으면 파츠 미착용(Order 0)으로 처리"))
	FSoftObjectPath AssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "동일 SlotType 내 노출 순서. 0 = 기본(파츠 없음)"))
	int32 AssetOrder = 0;
};
