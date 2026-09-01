// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Every log category the project uses. Declare here, define in TromboneLogs.cpp.
 */

/** Shared by all gimmicks - registration, activation, common flow. */
DECLARE_LOG_CATEGORY_EXTERN(LogGimmick, Log, All);

/** JazzBar drunkard NPC gimmick. */
DECLARE_LOG_CATEGORY_EXTERN(LogDrunkard, Log, All);

/** JazzBar beer flood gimmick. */
DECLARE_LOG_CATEGORY_EXTERN(LogBeerFlood, Log, All);
