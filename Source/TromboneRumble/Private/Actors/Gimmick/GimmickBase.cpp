// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/GimmickBase.h"

AGimmickBase::AGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGimmickBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	Deactivate();
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