// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/Breakable/BreakableProp.h"
#include "BreakableGlass.generated.h"

/** 술잔 / 와인잔. 폰이 닿기만 해도 부서지고 무기 공격에도 부서진다.
 *  콜리전 프로파일 BreakableGlass = QueryOnly + Pawn만 Overlap 이라 걸어가면 뚫린다 */
UCLASS()
class TROMBONERUMBLE_API ABreakableGlass : public ABreakableProp
{
	GENERATED_BODY()

public:
	ABreakableGlass();
};
