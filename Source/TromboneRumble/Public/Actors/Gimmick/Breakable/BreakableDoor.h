// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/Breakable/BreakableProp.h"
#include "BreakableDoor.generated.h"

/** 재즈바 문. 온전할 때는 벽처럼 막고, 취객이 등장할 때 UDrunkardDoorBreakerComponent가 부순다.
 *  오브젝트 타입이 WorldStatic이라 무기 스윕(Breakable 타입만 조회)에는 절대 걸리지 않는다 */
UCLASS()
class TROMBONERUMBLE_API ABreakableDoor : public ABreakableProp
{
	GENERATED_BODY()

public:
	ABreakableDoor();
};
