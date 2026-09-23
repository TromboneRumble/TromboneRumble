// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/GuideLine/GuideLine.h"
#include "Components/ActorComponents/GuideSignalComponent.h"
#include "NiagaraComponent.h"
#include "Utilities/TromboneLogs.h"

AGuideLine::AGuideLine()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGuideLine::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GuideFX = FindComponentByClass<UNiagaraComponent>();
	if (!GuideFX.IsValid())
	{
		UE_LOG(LogGimmick, Warning, TEXT("[GuideLine] %s has no Niagara component"), *GetName());
		return;
	}
	
	GuideFX->SetAutoActivate(false);
}

void AGuideLine::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer) return;

	const AActor* SourceActor = Source.LoadSynchronous();
	Signal = SourceActor ? SourceActor->FindComponentByClass<UGuideSignalComponent>() : nullptr;

	if (UGuideSignalComponent* GuideSignal = Signal.Get())
	{
		GuideSignal->RegisterLine(this);
	}
	else
	{
		UE_LOG(LogGimmick, Warning, TEXT("[GuideLine] %s has no Source with a guide signal, so it stays off"), *GetName());
	}
}

void AGuideLine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGuideSignalComponent* GuideSignal = Signal.Get())
	{
		GuideSignal->UnregisterLine(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AGuideLine::SetGuideVisible(const bool bVisible)
{
	UNiagaraComponent* FX = GuideFX.Get();
	if (!FX) return;

	if (bVisible)
	{
		FX->Activate(true);
	}
	else
	{
		FX->Deactivate();
	}
}
