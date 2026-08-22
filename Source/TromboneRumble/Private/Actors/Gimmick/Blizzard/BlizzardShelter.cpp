// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/Blizzard/BlizzardShelter.h"
#include "Actors/Gimmick/Blizzard/BlizzardGimmick.h"
#include "Actors/Gimmick/Blizzard/BlizzardEnvCopyUtil.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"

#if WITH_EDITOR
#include "ScopedTransaction.h"
#endif

ABlizzardShelter::ABlizzardShelter()
{
	PrimaryActorTick.bCanEverTick = false;

	SafeZone = CreateDefaultSubobject<USphereComponent>(TEXT("SafeZone"));
	SetRootComponent(SafeZone);

	// 판정은 거리 계산으로 하므로 물리 콜리전은 불필요. sphere 는 반경 저작/기즈모용.
	SafeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SafeZone->SetSphereRadius(400.f);

	ShelterLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ShelterLight"));
	ShelterLight->SetupAttachment(SafeZone);

	// 상태별 라이트 템플릿 (invisible + bAffectsWorld=false → 렌더/씬 등록 무관). 시드값은 기존 저작값.
	//  bEditableWhenInherited=false: Details 직접 편집을 잠근다. 값을 넣는 경로는 저장 버튼 하나뿐
	//  (ShelterLight 를 조정 → "전조/눈보라/평상시 상태 저장"). 코드에서의 쓰기는 이 게이트와 무관하다.
	auto SetupTemplate = [this](UPointLightComponent* Comp)
	{
		if (!Comp) return;
		Comp->SetupAttachment(SafeZone);
		Comp->SetVisibility(false);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->bAffectsWorld = false;
		Comp->bEditableWhenInherited = false;
	};

	WarningLightTemplate = CreateDefaultSubobject<UPointLightComponent>(TEXT("WarningLightTemplate"));
	ActiveLightTemplate  = CreateDefaultSubobject<UPointLightComponent>(TEXT("ActiveLightTemplate"));
	NormalLightTemplate  = CreateDefaultSubobject<UPointLightComponent>(TEXT("NormalLightTemplate"));
	SetupTemplate(WarningLightTemplate);
	SetupTemplate(ActiveLightTemplate);
	SetupTemplate(NormalLightTemplate);

	if (WarningLightTemplate) WarningLightTemplate->Intensity = 18000.f;  // 노을 배경이 밝아 높은 값이라야 빛이 보임
	if (ActiveLightTemplate)  ActiveLightTemplate->Intensity  = 3500.f;   // 어두운 눈보라 배경

	// 평상시는 미리보기가 전체 복사라 ShelterLight 상태를 빠짐없이 담아야 한다.
	// BP_BlizzardShelter 의 ShelterLight 저작값과 같은 값이다 — 거기가 바뀌면 여기도 갱신할 것.
	if (NormalLightTemplate)
	{
		NormalLightTemplate->Intensity         = 0.f;      // 평상시엔 꺼져 있다
		NormalLightTemplate->AttenuationRadius = 300.f;
		NormalLightTemplate->Temperature       = 2500.f;
		NormalLightTemplate->bUseTemperature   = true;
	}

	LightTargetState = EBlizzardState::Idle;
}

void ABlizzardShelter::BeginPlay()
{
	Super::BeginPlay();

	// 라이트 연출은 데디케이티드 서버엔 불필요.
	if (GetNetMode() == NM_DedicatedServer) return;
	if (!ShelterLight) return;

	// 평상시(배치된 라이트) 값 캡처 + 상태별 목표 구성 (평상시 위에 템플릿 오버라이드 얹기).
	NormalSnapshot  = Cast<UPointLightComponent>(FBlizzardEnvCopyUtil::CreateSnapshot(ShelterLight));
	StartSnapshot   = Cast<UPointLightComponent>(FBlizzardEnvCopyUtil::CreateSnapshot(ShelterLight));
	ResolvedWarning = Cast<UPointLightComponent>(FBlizzardEnvCopyUtil::CreateSnapshot(ShelterLight));
	ResolvedActive  = Cast<UPointLightComponent>(FBlizzardEnvCopyUtil::CreateSnapshot(ShelterLight));

	if (WarningLightTemplate) FBlizzardEnvCopyUtil::CopyOverriddenProperties(WarningLightTemplate, ResolvedWarning);
	if (ActiveLightTemplate)  FBlizzardEnvCopyUtil::CopyOverriddenProperties(ActiveLightTemplate, ResolvedActive);
}

bool ABlizzardShelter::IsLocationInsideShelter(const FVector& WorldLoc) const
{
	if (!SafeZone) return false;

	const float Radius = SafeZone->GetScaledSphereRadius();
	return FVector::DistSquared(SafeZone->GetComponentLocation(), WorldLoc) <= FMath::Square(Radius);
}

UPointLightComponent* ABlizzardShelter::GetLightTargetFor(EBlizzardState State) const
{
	switch (State)
	{
	case EBlizzardState::Warning:	return ResolvedWarning;
	case EBlizzardState::Active:	return ResolvedActive;
	case EBlizzardState::Idle:
	default:						return NormalSnapshot;
	}
}

void ABlizzardShelter::HandleBlizzardState(EBlizzardState NewState)
{
	if (!ShelterLight || !StartSnapshot) return;  // 스냅샷 미구성(데디) 시 스킵

	// 이전 페이드가 끝나기 전에 상태가 또 바뀔 수 있으므로, 현재 실값에서 이어 간다.
	FBlizzardEnvCopyUtil::CopyProperties(ShelterLight, StartSnapshot);
	LightTargetState = NewState;

	// 비보간(bool/enum) 프로퍼티는 전이 시점에 즉시 스냅.
	if (UPointLightComponent* Target = GetLightTargetFor(NewState))
	{
		if (FBlizzardEnvCopyUtil::ApplyNonLerpable(Target, ShelterLight))
		{
			ShelterLight->MarkRenderStateDirty();
		}
	}

	OnLightFadeRequested();
}

void ABlizzardShelter::UpdateLightFade(float Alpha)
{
	if (!ShelterLight || !StartSnapshot) return;

	UPointLightComponent* Target = GetLightTargetFor(LightTargetState);
	if (!Target) return;

	// Intensity/LightColor 는 프록시 재생성 없는 fast-path setter 로 (그림자 캐시 보존).
	FBlizzardEnvCopyUtil::ApplyHotProps(StartSnapshot, Target, ShelterLight, Alpha);

	if (FBlizzardEnvCopyUtil::LerpProperties(StartSnapshot, Target, ShelterLight, Alpha,
		&FBlizzardEnvCopyUtil::GetHotPropNames(ShelterLight)))
	{
		ShelterLight->MarkRenderStateDirty();
	}
}

#if WITH_EDITOR
UPointLightComponent* ABlizzardShelter::EditorPickStateTemplate(EBlizzardState State) const
{
	switch (State)
	{
	case EBlizzardState::Warning: return WarningLightTemplate;
	case EBlizzardState::Active:  return ActiveLightTemplate;
	default:                      return NormalLightTemplate;
	}
}

void ABlizzardShelter::EditorSaveState(EBlizzardState State)
{
	UPointLightComponent* Template = EditorPickStateTemplate(State);
	if (!ShelterLight || !Template) return;
	Template->Modify();
	FBlizzardEnvCopyUtil::CopyProperties(ShelterLight, Template);
}

void ABlizzardShelter::EditorLoadState(EBlizzardState State)
{
	UPointLightComponent* Template = EditorPickStateTemplate(State);
	if (!ShelterLight || !Template) return;
	EditorEnsureNormalBackup();
	ShelterLight->Modify();

	// 평상시만 전체 복사. 평상시 값의 상당수가 클래스 기본값과 같아서
	// 바뀐 값만 덮는 방식으로는 전조/눈보라 룩을 되돌릴 수 없다.
	const bool bChanged = (State == EBlizzardState::Idle)
		? FBlizzardEnvCopyUtil::CopyProperties(Template, ShelterLight)
		: FBlizzardEnvCopyUtil::CopyOverriddenProperties(Template, ShelterLight);
	if (bChanged)
	{
		ShelterLight->MarkRenderStateDirty();
	}
}

void ABlizzardShelter::EditorRestoreNormal()
{
	if (!ShelterLight || !EditorNormalBackup) return;
	ShelterLight->Modify();
	FBlizzardEnvCopyUtil::CopyProperties(EditorNormalBackup, ShelterLight);
	ShelterLight->MarkRenderStateDirty();
}

void ABlizzardShelter::EditorEnsureNormalBackup()
{
	if (EditorNormalBackup || !ShelterLight) return;
	EditorNormalBackup = Cast<UPointLightComponent>(FBlizzardEnvCopyUtil::CreateSnapshot(ShelterLight));
}

void ABlizzardShelter::SaveWarningFromWorld()
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld()) return;
	FScopedTransaction Tx(NSLOCTEXT("BlizzardShelter", "SaveWarning", "쉘터 예고 상태 저장"));
	EditorSaveState(EBlizzardState::Warning);
}

void ABlizzardShelter::SaveActiveFromWorld()
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld()) return;
	FScopedTransaction Tx(NSLOCTEXT("BlizzardShelter", "SaveActive", "쉘터 눈보라 상태 저장"));
	EditorSaveState(EBlizzardState::Active);
}

void ABlizzardShelter::SaveNormalFromWorld()
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld()) return;
	FScopedTransaction Tx(NSLOCTEXT("BlizzardShelter", "SaveNormal", "쉘터 평상시 상태 저장"));
	EditorSaveState(EBlizzardState::Idle);
}

void ABlizzardShelter::LoadWarningToWorld()
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld()) return;
	FScopedTransaction Tx(NSLOCTEXT("BlizzardShelter", "LoadWarning", "쉘터 예고 상태 미리보기"));
	EditorLoadState(EBlizzardState::Warning);
}

void ABlizzardShelter::LoadActiveToWorld()
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld()) return;
	FScopedTransaction Tx(NSLOCTEXT("BlizzardShelter", "LoadActive", "쉘터 눈보라 상태 미리보기"));
	EditorLoadState(EBlizzardState::Active);
}

void ABlizzardShelter::LoadNormalToWorld()
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld()) return;
	FScopedTransaction Tx(NSLOCTEXT("BlizzardShelter", "LoadNormal", "쉘터 평상시 상태 미리보기"));
	EditorLoadState(EBlizzardState::Idle);
}

void ABlizzardShelter::RestoreNormalToWorld()
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld()) return;
	if (!EditorNormalBackup) return;
	FScopedTransaction Tx(NSLOCTEXT("BlizzardShelter", "RestoreNormal", "쉘터 평상시 복원"));
	EditorRestoreNormal();
}
#endif // WITH_EDITOR
