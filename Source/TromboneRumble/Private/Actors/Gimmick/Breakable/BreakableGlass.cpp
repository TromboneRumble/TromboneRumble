// Copyright (C) 2026 biksari studio. All Rights Reserved.


#include "Actors/Gimmick/Breakable/BreakableGlass.h"
#include "Components/StaticMeshComponent.h"

ABreakableGlass::ABreakableGlass()
{
	bBreakOnPawnTouch = true;
	bBreakOnAttack = true;

	if (IntactMesh)
	{
		IntactMesh->SetCollisionProfileName(TEXT("BreakableGlass"));
		IntactMesh->SetGenerateOverlapEvents(true);
	}
}
