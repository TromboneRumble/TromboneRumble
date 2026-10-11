// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/BonusGimmickConfig.h"
#include "PresentGimmickConfig.generated.h"

/**
 * Settings of the present of the snow field, dropped by ABonusSpawner.
 *
 * @see ABonusSpawner
 * @see APresent
 */
UCLASS(meta = (DisplayName = "선물"))
class TROMBONERUMBLE_API UPresentGimmickConfig : public UBonusGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UPresentGimmickConfig() { GimmickType = EGimmickType::Present; }
};
