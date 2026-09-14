// Copyright (C) 2026 biksari studio. All Rights Reserved.


#include "Actors/Gimmick/Blizzard/BlizzardGuideLine.h"
#include "Actors/Gimmick/Blizzard/BlizzardShelter.h"
#include "NiagaraComponent.h"
#include "Utilities/TromboneLogs.h"

ABlizzardGuideLine::ABlizzardGuideLine()
{
	PrimaryActorTick.bCanEverTick = false;

	// 스플라인/나이아가라는 BP 소유다. 여기서 네이티브 컴포넌트를 만들면 BP 루트가 한 단 밀린다.
}

void ABlizzardGuideLine::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GuideFX = FindComponentByClass<UNiagaraComponent>();
	if (!GuideFX.IsValid())
	{
		UE_LOG(LogGimmick, Warning, TEXT("[안내선] %s 에 나이아가라 컴포넌트가 없습니다"), *GetName());
		return;
	}

	// 자동 활성화는 InitializeComponents 에서 걸린다 (Actor.cpp:5718). 그보다 앞인 여기서 꺼야
	// 레벨 시작 시 한 프레임도 안 보인다 - PostInitializeComponents 나 BeginPlay 는 이미 늦다.
	GuideFX->SetAutoActivate(false);
}

void ABlizzardGuideLine::BeginPlay()
{
	Super::BeginPlay();

	// 순수 연출이라 데디케이티드 서버에는 불필요.
	if (GetNetMode() == NM_DedicatedServer) return;

	ResolvedShelter = TargetShelter.IsValid() ? TargetShelter.Get() : TargetShelter.LoadSynchronous();

	if (ABlizzardShelter* Shelter = ResolvedShelter.Get())
	{
		Shelter->OnDoorChanged.AddDynamic(this, &ThisClass::HandleShelterDoorChanged);
		bDoorOpen = Shelter->IsDoorOpen();
	}
	else
	{
		UE_LOG(LogGimmick, Warning, TEXT("[안내선] %s 에 목표 천막이 없어 켜지지 않습니다"), *GetName());
	}

	if (ABlizzardGimmick* Gimmick = BlizzardGimmick.Get())
	{
		Gimmick->OnBlizzardStateChangedDelegate.AddDynamic(this, &ThisClass::HandleBlizzardStateChanged);
		CachedState = Gimmick->GetBlizzardState();
	}
	else
	{
		UE_LOG(LogGimmick, Warning, TEXT("[안내선] %s 에 눈보라 기믹이 없어 켜지지 않습니다"), *GetName());
	}

	// 복제 도착과 BeginPlay 순서를 믿지 않는다. 현재 값으로 한 번 맞춰 두고 시작.
	RefreshGuideLine();
}

void ABlizzardGuideLine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ABlizzardShelter* Shelter = ResolvedShelter.Get())
	{
		Shelter->OnDoorChanged.RemoveDynamic(this, &ThisClass::HandleShelterDoorChanged);
	}

	if (ABlizzardGimmick* Gimmick = BlizzardGimmick.Get())
	{
		Gimmick->OnBlizzardStateChangedDelegate.RemoveDynamic(this, &ThisClass::HandleBlizzardStateChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void ABlizzardGuideLine::HandleBlizzardStateChanged(EBlizzardState NewState)
{
	CachedState = NewState;
	RefreshGuideLine();
}

void ABlizzardGuideLine::HandleShelterDoorChanged(bool bIsOpen)
{
	bDoorOpen = bIsOpen;
	RefreshGuideLine();
}

void ABlizzardGuideLine::RefreshGuideLine()
{
	UNiagaraComponent* FX = GuideFX.Get();
	if (!FX) return;

	// 전조이면서 이 천막의 문이 열렸을 때만 켠다. 두 신호 중 뭐가 먼저 왔는지는 보지 않는다.
	const bool bShouldShow = (CachedState == EBlizzardState::Warning) && bDoorOpen;
	if (bShouldShow == bShowing) return;

	bShowing = bShouldShow;

	if (bShouldShow)
	{
		FX->Activate(true);
		return;
	}

	// 전조 -> 눈보라에서만 부드럽게. Deactivate 는 새 스폰만 막아서 이미 떠난 파티클이
	// 천막까지 날아간다 (NiagaraComponent.cpp:1515, 실행 상태 Inactive - NiagaraTypes.h:567).
	if (CachedState == EBlizzardState::Active)
	{
		FX->Deactivate();
	}
	else
	{
		FX->DeactivateImmediate();
	}
}
