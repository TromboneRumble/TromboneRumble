// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/GimmickBase.h"

AGimmickBase::AGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGimmickBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Deactivate();
	
	Super::EndPlay(EndPlayReason);
}

void AGimmickBase::Activate()
{
	if (bIsActive) return;
	
	bIsActive = true;
}

void AGimmickBase::Deactivate()
{
	if (!bIsActive) return;
	
	bIsActive = false;
	GetWorldTimerManager().ClearAllTimersForObject(this);
}