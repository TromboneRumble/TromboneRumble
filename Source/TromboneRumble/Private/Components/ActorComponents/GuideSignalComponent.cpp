// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/GuideSignalComponent.h"
#include "Actors/Gimmick/GuideLine/GuideLine.h"
#include "Net/UnrealNetwork.h"

UGuideSignalComponent::UGuideSignalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGuideSignalComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bGuiding);
}

void UGuideSignalComponent::SetGuiding(const bool bInGuiding)
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || bGuiding == bInGuiding) return;

	bGuiding = bInGuiding;
	OnRep_Guiding();
}

void UGuideSignalComponent::OnRep_Guiding()
{
	ApplyToLines();
}

void UGuideSignalComponent::RegisterLine(AGuideLine* Line)
{
	if (!Line) return;

	Lines.AddUnique(Line);
	Line->SetGuideVisible(bGuiding);
}

void UGuideSignalComponent::UnregisterLine(AGuideLine* Line)
{
	Lines.RemoveSingleSwap(Line);
}

void UGuideSignalComponent::ApplyToLines() const
{
	for (const TWeakObjectPtr<AGuideLine>& LinePtr : Lines)
	{
		if (AGuideLine* Line = LinePtr.Get())
		{
			Line->SetGuideVisible(bGuiding);
		}
	}
}
