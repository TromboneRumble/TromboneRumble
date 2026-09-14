// Copyright (C) 2026 biksari studio. All Rights Reserved.


#include "Actors/Gimmick/Breakable/BreakableDoor.h"
#include "Components/StaticMeshComponent.h"

ABreakableDoor::ABreakableDoor()
{
	bBreakOnPawnTouch = false;
	bBreakOnAttack = false;

	if (IntactMesh)
	{
		IntactMesh->SetCollisionProfileName(TEXT("BreakableDoor"));
		IntactMesh->SetGenerateOverlapEvents(false);
	}
}
