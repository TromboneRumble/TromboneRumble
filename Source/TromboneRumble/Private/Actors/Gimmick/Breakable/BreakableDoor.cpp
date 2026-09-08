// Copyright (C) 2026 biksari studio. All Rights Reserved.


#include "Actors/Gimmick/Breakable/BreakableDoor.h"
#include "Components/StaticMeshComponent.h"

ABreakableDoor::ABreakableDoor()
{
	bBreakOnPawnTouch = false;
	bBreakOnAttack = false;

	// 문은 조각이 크고 무거우므로 파편도 오래 남긴다
	DebrisLifetimeSeconds = 20.f;
	DebrisImpulseRadius = 250.f;

	if (IntactMesh)
	{
		IntactMesh->SetCollisionProfileName(TEXT("BreakableDoor"));
	}
}
