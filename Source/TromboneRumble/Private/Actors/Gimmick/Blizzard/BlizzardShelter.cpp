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
	//  (ShelterLight 를 조정 → "전조/눈보라 상태 저장"). 코드에서의 쓰기는 이 게이트와 무관하다.
	auto SetupTemplate = [this](UPointLightComponent* Comp, float Intensity)
	{
		if (!Comp) return;
		Comp->SetupAttachment(SafeZone);
		Comp->SetVisibility(false);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->bAffectsWorld = false;
		Comp->bEditableWhenInherited = false;
		Comp->Intensity = Intensity;
	};

	WarningLightTemplate = CreateDefaultSubobject<UPointLightComponent>(TEXT("WarningLightTemplate"));
	ActiveLightTemplate  = CreateDefaultSubobject<UPointLightComponent>(TEXT("ActiveLightTemplate"));
	SetupTemplate(WarningLightTemplate, 18000.f);  // 노을 배경이 밝아 높은 값이라야 빛이 보임
	SetupTemplate(ActiveLightTemplate, 3500.f);    // 어두운 눈보라 배경

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
void ABlizzardShelter::EditorSaveState(EBlizzardState State)
{
	UPointLightComponent* Template = (State == EBlizzardState::Warning) ? WarningLightTemplate : ActiveLightTemplate;
	if (!ShelterLight || !Template) return;
	Template->Modify();
	FBlizzardEnvCopyUtil::CopyProperties(ShelterLight, Template);
}

void ABlizzardShelter::EditorLoadState(EBlizzardState State)
{
	UPointLightComponent* Template = (State == EBlizzardState::Warning) ? WarningLightTemplate : ActiveLightTemplate;
	if (!ShelterLight || !Template) return;
	EditorEnsureNormalBackup();
	ShelterLight->Modify();
	if (FBlizzardEnvCopyUtil::CopyOverriddenProperties(Template, ShelterLight))
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

void ABlizzardShelter::RestoreNormalToWorld()
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld()) return;
	if (!EditorNormalBackup) return;
	FScopedTransaction Tx(NSLOCTEXT("BlizzardShelter", "RestoreNormal", "쉘터 평상시 복원"));
	EditorRestoreNormal();
}
#endif // WITH_EDITOR
