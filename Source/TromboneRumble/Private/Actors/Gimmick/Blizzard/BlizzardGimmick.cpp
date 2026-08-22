// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/Blizzard/BlizzardGimmick.h"
#include "Actors/Gimmick/Blizzard/BlizzardShelter.h"
#include "Actors/Gimmick/Blizzard/BlizzardEnvCopyUtil.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Interfaces/CombatReceiver.h"
#include "Utilities/Defines.h"
#include "Utilities/DebugHelper.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "EngineUtils.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "MaterialTypes.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
#include "AkSwitchValue.h"
#include "Net/UnrealNetwork.h"

#if WITH_EDITOR
#include "ScopedTransaction.h"
#endif

namespace BlizzardEnvParams
{
	// 눈 나이아가라 시스템의 사용자 노출 파라미터
	static const FName SnowSpawnRateScale(TEXT("SnowSpawnRateScale"));
	static const FName WindVelocity(TEXT("WindVelocity"));

	// 볼류메트릭 클라우드 머티리얼의 스칼라 파라미터
	static const FName Coverage(TEXT("Coverage"));
}

ABlizzardGimmick::ABlizzardGimmick()
{
	// Active 동안 서버에서만 켜서 프레임 단위 밀기에 사용
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	// 맵 전역 이벤트이므로 위치 기반 relevancy 컬링으로 클라가 상태 복제를 놓치지 않게 함
	bAlwaysRelevant = true;

	GimmickType = EGimmickType::Blizzard;

	// 라이팅 템플릿을 붙일 네이티브 루트. (앰비언스는 Ak 컴포넌트 없이 글로벌 오브젝트로 나간다)
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// 상태별 라이팅 템플릿. 상태값을 담아두는 "데이터 컨테이너" 컴포넌트다.
	//  - invisible + (라이트는) bAffectsWorld=false → 렌더 프록시/맵체크 경고/씬 등록에 전혀 관여하지 않는다.
	//  - 레벨에 배치된(디스크 로드) 기믹의 경우 SkyLight 템플릿은 PostLoad 가 invisible 이라 캡처 큐에서 빠진다.
	//  - 시드값은 기존 flat UPROPERTY 저작값과 동일 → 최초 룩 보존. 템플릿에서 바뀐 값만 이후 구동된다.
	//  - bEditableWhenInherited=false: Details 직접 편집을 잠근다. 값을 넣는 경로는 저장 버튼(월드→템플릿) 하나뿐.
	//    (코드에서의 쓰기는 이 게이트와 무관하므로 저장 버튼은 정상 동작 — ActorComponent.cpp:2417 IsEditableWhenInherited)
	auto SetupLightTemplate = [this](USceneComponent* Comp)
	{
		if (!Comp) return;
		Comp->SetupAttachment(SceneRoot);
		Comp->SetVisibility(false);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->bEditableWhenInherited = false;
	};

	// 시드값은 필드에 직접 대입한다 (생성자 내 setter 는 등록 전이라 부작용/얼리아웃 소지). LightColor 는 FColor.
	WarningSunTemplate = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("WarningSunTemplate"));
	ActiveSunTemplate  = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("ActiveSunTemplate"));
	if (WarningSunTemplate)
	{
		SetupLightTemplate(WarningSunTemplate);
		WarningSunTemplate->bAffectsWorld = false;
		WarningSunTemplate->Intensity  = 2.5f;                                                  // 저녁 노을: 낮은 밝기
		WarningSunTemplate->LightColor = FLinearColor(1.0f, 0.55f, 0.28f, 1.f).ToFColor(true);  // 따뜻한 주황 노을빛
	}
	if (ActiveSunTemplate)
	{
		SetupLightTemplate(ActiveSunTemplate);
		ActiveSunTemplate->bAffectsWorld = false;
		ActiveSunTemplate->Intensity  = 0.3f;                                                   // 눈보라: 거의 꺼짐
		ActiveSunTemplate->LightColor = FLinearColor(0.55f, 0.68f, 0.9f, 1.f).ToFColor(true);   // 차가운 청회색
	}

	WarningFogTemplate = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("WarningFogTemplate"));
	ActiveFogTemplate  = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ActiveFogTemplate"));
	if (WarningFogTemplate)
	{
		SetupLightTemplate(WarningFogTemplate);
		WarningFogTemplate->FogDensity = 0.05f;
		WarningFogTemplate->FogInscatteringLuminance = FLinearColor(0.95f, 0.5f, 0.35f, 1.f);   // 노을빛 따뜻한 안개
	}
	if (ActiveFogTemplate)
	{
		SetupLightTemplate(ActiveFogTemplate);
		ActiveFogTemplate->FogDensity = 0.3f;
		ActiveFogTemplate->FogInscatteringLuminance = FLinearColor(0.07f, 0.09f, 0.14f, 1.f);   // 차갑고 어두운 눈보라 안개
	}

	WarningAtmosphereTemplate = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("WarningAtmosphereTemplate"));
	ActiveAtmosphereTemplate  = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("ActiveAtmosphereTemplate"));
	if (WarningAtmosphereTemplate)
	{
		SetupLightTemplate(WarningAtmosphereTemplate);
		WarningAtmosphereTemplate->SkyLuminanceFactor = FLinearColor(1.0f, 0.65f, 0.45f, 1.f);  // 노을빛 하늘
	}
	if (ActiveAtmosphereTemplate)
	{
		SetupLightTemplate(ActiveAtmosphereTemplate);
		ActiveAtmosphereTemplate->SkyLuminanceFactor = FLinearColor(0.12f, 0.16f, 0.26f, 1.f);  // 어둡고 차가운 하늘
	}

	WarningSkyLightTemplate = CreateDefaultSubobject<USkyLightComponent>(TEXT("WarningSkyLightTemplate"));
	ActiveSkyLightTemplate  = CreateDefaultSubobject<USkyLightComponent>(TEXT("ActiveSkyLightTemplate"));
	if (WarningSkyLightTemplate)
	{
		SetupLightTemplate(WarningSkyLightTemplate);
		WarningSkyLightTemplate->bAffectsWorld = false;
		WarningSkyLightTemplate->Intensity  = 0.8f;                                                  // 노을: 음영이 죽지 않게
		WarningSkyLightTemplate->LightColor = FLinearColor(1.0f, 0.72f, 0.55f, 1.f).ToFColor(true);  // 따뜻한 노을 환경광
	}
	if (ActiveSkyLightTemplate)
	{
		SetupLightTemplate(ActiveSkyLightTemplate);
		ActiveSkyLightTemplate->bAffectsWorld = false;
		ActiveSkyLightTemplate->Intensity  = 0.15f;                                                  // 눈보라: 전반적으로 어둡게
		ActiveSkyLightTemplate->LightColor = FLinearColor(0.6f, 0.72f, 0.92f, 1.f).ToFColor(true);   // 차가운 눈보라 환경광
	}

	// 평상시 템플릿. 미리보기가 전체 복사라 여기 없는 값은 클래스 기본값으로 리셋된다 →
	// InGame_SnowField 라이팅 액터의 기본값과 다른 프로퍼티를 빠짐없이 시드해야 한다.
	// 대기/스카이라이트는 실제로 전부 기본값이라 시드가 없는 것이 맞다.
	NormalSunTemplate        = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("NormalSunTemplate"));
	NormalFogTemplate        = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("NormalFogTemplate"));
	NormalAtmosphereTemplate = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("NormalAtmosphereTemplate"));
	NormalSkyLightTemplate   = CreateDefaultSubobject<USkyLightComponent>(TEXT("NormalSkyLightTemplate"));
	if (NormalSunTemplate)
	{
		SetupLightTemplate(NormalSunTemplate);
		NormalSunTemplate->bAffectsWorld = false;
		NormalSunTemplate->Intensity                         = 6.f;         // 평상시 태양 (색은 흰색 = 기본값)
		NormalSunTemplate->bUseTemperature                    = true;
		NormalSunTemplate->LightSourceAngle                   = 0.7357f;
		NormalSunTemplate->DynamicShadowDistanceMovableLight  = 20000.f;
		NormalSunTemplate->DistanceFieldShadowDistance        = 30000.f;
		NormalSunTemplate->bEnableLightShaftOcclusion         = true;
		NormalSunTemplate->bUseRayTracedDistanceFieldShadows  = false;
		NormalSunTemplate->LightFunctionScale                 = FVector(4235.453613f);
		NormalSunTemplate->LightFunctionFadeDistance          = 77466.1875f;
		NormalSunTemplate->DisabledBrightness                 = 0.696f;
	}
	if (NormalFogTemplate)
	{
		SetupLightTemplate(NormalFogTemplate);
		NormalFogTemplate->FogDensity = 0.0436f;   // 인스캐터링 색은 검정 = 기본값
	}
	if (NormalAtmosphereTemplate)
	{
		SetupLightTemplate(NormalAtmosphereTemplate);
	}
	if (NormalSkyLightTemplate)
	{
		SetupLightTemplate(NormalSkyLightTemplate);
		NormalSkyLightTemplate->bAffectsWorld = false;
	}
}
void ABlizzardGimmick::Activate()
{
	const bool bWasActive = IsActive();

	Super::Activate();

	if (!bWasActive && HasAuthority())
	{
		ScheduleNextBlizzard();
	}
}

void ABlizzardGimmick::Deactivate()
{
	if (HasAuthority())
	{
		SetActorTickEnabled(false);
		PushTargets.Reset();
		RemoveAllSlows();
		ExposureTimeMap.Empty();
		CloseAllShelterDoors();   // 기믹이 도중에 꺼져도 문이 열린 채 남지 않게
		SetState(EBlizzardState::Idle);
	}

	// Super가 bIsActive=false + ClearAllTimersForObject 로 3개 타이머를 정리한다.
	Super::Deactivate();
}

void ABlizzardGimmick::BeginPlay()
{
	Super::BeginPlay();

	// 팅겨낼 목적지는 게임플레이라 데디 서버에도 필요하다 → 아래 데디 리턴보다 위.
	GatherPlayerStarts();

	// 라이팅/VFX 는 순수 연출이라 데디케이티드 서버에는 불필요.
	if (GetNetMode() == NM_DedicatedServer) return;

	InitializeBlizzardComponents();
	BuildDrivenEnvEntries();
	StartBlizzardAmbience();
}

void ABlizzardGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopBlizzardAmbience();

	Super::EndPlay(EndPlayReason);
}

void ABlizzardGimmick::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || BlizzardState != EBlizzardState::Active) return;

	const FVector WindDir = WindDirection;
	if (WindDir.IsNearlyZero()) return;

	// 바람을 위치 오프셋으로 적용해 플레이어 이동과 "합성"한다 (Velocity를 건드리지 않음).
	// 속도를 직접 수정하면 CMC 브레이킹/최대속도 클램프와 싸우게 되어,
	// 정지자를 밀 수 있는 세기는 반드시 이동 입력의 저항도 압도해버린다.
	// 위치 오프셋은 CMC 계산 결과 위에 얹히므로: 정지 시 PushSpeed 로 밀리고,
	// 역풍 이동 시 (이동속도 - PushSpeed) 로 저항하며 전진할 수 있다.
	for (const TWeakObjectPtr<ADefaultTromboneCharacter>& TargetPtr : PushTargets)
	{
		ADefaultTromboneCharacter* Target = TargetPtr.Get();
		if (!IsValid(Target)) continue;
		if (Target->IsRagdoll()) continue;  // 래그돌 굴리기는 TickExposure 가 담당

		const UCharacterMovementComponent* Move = Target->GetCharacterMovement();
		if (!Move) continue;

		// 기준 속도 = 현재 걷기 속도. MaxWalkSpeed 에는 슬로우/버프/악기장착이 이미 반영돼 있고,
		// 스프린트 중에는 SprintSpeed 로 바뀌므로 Walk/Sprint 비로 걷기 기준 환산
		// (바람이 스프린트에 비례 강화되지 않아야 달리기가 확실한 탈출 수단이 된다)
		float ReferenceSpeed = Move->MaxWalkSpeed;
		const UCharacterDataAsset* Data = Target->GetCharacterDataAsset();
		if (Data && Target->IsSprinting() && Data->SprintSpeed > KINDA_SMALL_NUMBER)
		{
			ReferenceSpeed *= Data->WalkSpeed / Data->SprintSpeed;
		}

		const float PushSpeed = ReferenceSpeed * WindPushRatio;

		// sweep=true: 벽/다른 캐릭터를 뚫고 밀리지 않게 함
		Target->AddActorWorldOffset(WindDir * PushSpeed * DeltaSeconds, true);
	}
}

void ABlizzardGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BlizzardState);
	DOREPLIFETIME(ThisClass, WindDirection);
}


void ABlizzardGimmick::ScheduleNextBlizzard()
{
	if (!HasAuthority()) return;

	const float NextInterval = FMath::RandRange(MinIntervalSeconds, MaxIntervalSeconds);

	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	GetWorldTimerManager().SetTimer(
		ScheduleTimerHandle,
		this,
		&ThisClass::StartWarning,
		NextInterval,
		false
	);
}

void ABlizzardGimmick::StartWarning()
{
	if (!HasAuthority()) return;

	// 방향을 먼저 확정해 SetState 와 같은 번치로 복제되게 함
	const FVector Axis = WindAxis.GetSafeNormal2D();
	WindDirection = Axis * (FMath::RandBool() ? 1.f : -1.f);

	SetState(EBlizzardState::Warning);

	// 대피처를 알려주는 것이 전조의 역할이므로 문은 여기서 열린다.
	// 쉘터는 레벨 배치 액터라 Warning~Active 사이에 목록이 변하지 않는다 → 여기서 한 번만 수집.
	GatherShelters();
	OpenRandomShelterDoors();

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&ThisClass::StartBlizzard,
		WarningDuration,
		false
	);
}

void ABlizzardGimmick::StartBlizzard()
{
	if (!HasAuthority()) return;

	ExposureTimeMap.Empty();
	PushTargets.Reset();

	SetState(EBlizzardState::Active);
	SetActorTickEnabled(true);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&ThisClass::EndBlizzard,
		ActiveDuration,
		false
	);

	GetWorldTimerManager().ClearTimer(ExposureTimerHandle);
	GetWorldTimerManager().SetTimer(
		ExposureTimerHandle,
		this,
		&ThisClass::TickExposure,
		ExposureCheckInterval,
		true
	);
}

void ABlizzardGimmick::EndBlizzard()
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(ExposureTimerHandle);

	SetActorTickEnabled(false);
	PushTargets.Reset();
	RemoveAllSlows();
	ExposureTimeMap.Empty();

	// 순서 주의: 내보내기가 먼저다. 문을 닫고 나면 IsSheltering() 이 false 라 아무도 안 잡힌다.
	EjectCharactersFromShelters();
	CloseAllShelterDoors();

	SetState(EBlizzardState::Idle);

	ScheduleNextBlizzard();
}


void ABlizzardGimmick::InitializeBlizzardComponents()
{
	if (const ANiagaraActor* SnowActor = SnowVFXActor.LoadSynchronous())
	{
		SnowNiagaraComponent = SnowActor->GetNiagaraComponent();
	}

	if (const ADirectionalLight* Sun = SunLight.LoadSynchronous())
	{
		// ADirectionalLight::GetComponent() 는 WITH_EDITORONLY_DATA 안에 있어 쿠킹 빌드가 깨진다. 베이스 접근자를 쓸 것.
		SunComponent = Cast<UDirectionalLightComponent>(Sun->GetLightComponent());
	}

	if (const AExponentialHeightFog* Fog = HeightFog.LoadSynchronous())
	{
		FogComponent = Fog->GetComponent();
	}

	if (const ASkyAtmosphere* Atmo = SkyAtmosphere.LoadSynchronous())
	{
		AtmosphereComponent = Atmo->GetComponent();
	}

	if (const ASkyLight* Sky = SkyLightActor.LoadSynchronous())
	{
		SkyLightComponent = Sky->GetLightComponent();
	}

	// AVolumetricCloud 에는 컴포넌트 접근자가 없어 직접 찾는다.
	if (const AVolumetricCloud* Cloud = CloudActor.LoadSynchronous())
	{
		UVolumetricCloudComponent* CloudComp = Cloud->FindComponentByClass<UVolumetricCloudComponent>();
		UMaterialInterface* CloudMaterial = CloudComp ? CloudComp->GetMaterial() : nullptr;
		if (CloudMaterial)
		{
			// 커버리지를 런타임에 바꾸려면 MID 가 필요하므로 만들어서 도로 꽂아준다.
			CloudMID = UMaterialInstanceDynamic::Create(CloudMaterial, this);
			if (CloudMID)
			{
				CloudComp->SetMaterial(CloudMID);
			}
		}
	}

	// 쉘터 라이트 연출용. 게임플레이 판정용 Shelters 는 서버가 StartBlizzard 마다 따로 수집한다.
	CachedBlizzardShelters.Reset();
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ABlizzardShelter> It(World); It; ++It)
		{
			if (ABlizzardShelter* Shelter = *It)
			{
				CachedBlizzardShelters.Add(Shelter);
			}
		}
	}
}

void ABlizzardGimmick::BuildDrivenEnvEntries()
{
	DrivenEnvComponents.Reset();

	// 라이브 컴포넌트 ↔ (전조 템플릿, 눈보라 템플릿) 페어링. 참조가 비면 해당 엔트리는 건너뛴다.
	AddDrivenEntry(SunComponent.Get(),        WarningSunTemplate,        ActiveSunTemplate);
	AddDrivenEntry(FogComponent.Get(),        WarningFogTemplate,        ActiveFogTemplate);
	AddDrivenEntry(AtmosphereComponent.Get(), WarningAtmosphereTemplate, ActiveAtmosphereTemplate);
	AddDrivenEntry(SkyLightComponent.Get(),   WarningSkyLightTemplate,   ActiveSkyLightTemplate);

	// 구름 커버리지(raw) 평상시 목표값 캡처.
	if (CloudMID)
	{
		NormalCoverage = CloudMID->K2_GetScalarParameterValue(BlizzardEnvParams::Coverage);
	}
}

void ABlizzardGimmick::AddDrivenEntry(USceneComponent* Live, USceneComponent* WarnTemplate, USceneComponent* ActiveTemplate)
{
	if (!Live) return;

	FBlizzardDrivenEnv Entry;
	Entry.Live = Live;
	Entry.WarningTemplate = WarnTemplate;
	Entry.ActiveTemplate = ActiveTemplate;

	// 평상시(레벨 저작) 값 캡처 + 블렌드 시작 스냅샷 초기화.
	Entry.NormalSnapshot = FBlizzardEnvCopyUtil::CreateSnapshot(Live);
	Entry.StartSnapshot  = FBlizzardEnvCopyUtil::CreateSnapshot(Live);

	// 상태별 목표 = 평상시 위에 템플릿의 "오버라이드된(=클래스 기본값과 다른)" 프로퍼티만 얹기.
	// 아티스트가 템플릿에서 바꾼 값만 구동되고, 안 건드린 값은 평상시 그대로 유지된다.
	Entry.ResolvedWarning = FBlizzardEnvCopyUtil::CreateSnapshot(Live);
	Entry.ResolvedActive  = FBlizzardEnvCopyUtil::CreateSnapshot(Live);
	if (WarnTemplate)   FBlizzardEnvCopyUtil::CopyOverriddenProperties(WarnTemplate, Entry.ResolvedWarning);
	if (ActiveTemplate) FBlizzardEnvCopyUtil::CopyOverriddenProperties(ActiveTemplate, Entry.ResolvedActive);

	DrivenEnvComponents.Add(MoveTemp(Entry));
}

USceneComponent* ABlizzardGimmick::GetEnvTargetFor(const FBlizzardDrivenEnv& Entry, EBlizzardState State) const
{
	switch (State)
	{
	case EBlizzardState::Warning:	return Entry.ResolvedWarning;
	case EBlizzardState::Active:	return Entry.ResolvedActive;
	case EBlizzardState::Idle:
	default:						return Entry.NormalSnapshot;
	}
}

float ABlizzardGimmick::GetStateCoverage(EBlizzardState State) const
{
	switch (State)
	{
	case EBlizzardState::Warning:	return WarningCoverage;
	case EBlizzardState::Active:	return FrozenCoverage;
	case EBlizzardState::Idle:
	default:						return NormalCoverage;
	}
}

void ABlizzardGimmick::OnRep_BlizzardState()
{
	HandleBlizzardStateChanged(BlizzardState, WindDirection);
}

void ABlizzardGimmick::HandleBlizzardStateChanged(EBlizzardState NewState, const FVector& InWindDir)
{
	if (GetNetMode() == NM_DedicatedServer) return;

	// 이전 페이드가 끝나기 전에 상태가 또 바뀔 수 있으므로, 현재 실값에서 이어 간다 (BeginEnvBlend 가 재캡처).
	BeginEnvBlend(NewState);

	ApplySnowStorm(NewState, InWindDir);
	ApplyAmbienceSwitch(NewState);

	for (const TWeakObjectPtr<ABlizzardShelter>& ShelterPtr : CachedBlizzardShelters)
	{
		if (ABlizzardShelter* Shelter = ShelterPtr.Get())
		{
			Shelter->HandleBlizzardState(NewState);
		}
	}

	// BP: FrozenTL PlayFromStart + 사운드 연출. 타임라인 Update 가 UpdateEnvironmentBlend 를 돌린다.
	OnBlizzardStateChanged(NewState, InWindDir);
}

void ABlizzardGimmick::BeginEnvBlend(EBlizzardState NewState)
{
	BlendTargetState = NewState;

	for (FBlizzardDrivenEnv& Entry : DrivenEnvComponents)
	{
		USceneComponent* Live = Entry.Live.Get();
		if (!Live) continue;

		// 시작 스냅샷 = 현재 실값 (전이마다 재캡처 → 페이드 중 전이도 매끄럽게 이어짐).
		FBlizzardEnvCopyUtil::CopyProperties(Live, Entry.StartSnapshot);

		// 비보간(bool/enum) 프로퍼티는 전이 시점에 목표값으로 즉시 스냅.
		if (USceneComponent* Target = GetEnvTargetFor(Entry, NewState))
		{
			if (FBlizzardEnvCopyUtil::ApplyNonLerpable(Target, Live))
			{
				Live->MarkRenderStateDirty();
			}
		}
	}

	BlendStartCoverage = CloudMID ? CloudMID->K2_GetScalarParameterValue(BlizzardEnvParams::Coverage) : 0.f;
}

void ABlizzardGimmick::UpdateEnvironmentBlend(float Alpha)
{
	for (FBlizzardDrivenEnv& Entry : DrivenEnvComponents)
	{
		USceneComponent* Live = Entry.Live.Get();
		USceneComponent* Start = Entry.StartSnapshot;
		USceneComponent* Target = GetEnvTargetFor(Entry, BlendTargetState);
		if (!Live || !Start || !Target) continue;

		// 핫 프로퍼티(Intensity/LightColor 등)는 프록시 재생성 없는 fast-path setter 로 (기존과 동일 성능).
		FBlizzardEnvCopyUtil::ApplyHotProps(Start, Target, Live, Alpha);

		// 나머지 보간 가능 프로퍼티는 직접 기록 후, 실제로 바뀐 게 있으면 한 번만 MarkRenderStateDirty.
		if (FBlizzardEnvCopyUtil::LerpProperties(Start, Target, Live, Alpha, &FBlizzardEnvCopyUtil::GetHotPropNames(Live)))
		{
			Live->MarkRenderStateDirty();
		}
	}

	if (CloudMID)
	{
		CloudMID->SetScalarParameterValue(
			BlizzardEnvParams::Coverage,
			FMath::Lerp(BlendStartCoverage, GetStateCoverage(BlendTargetState), Alpha));
	}
}

void ABlizzardGimmick::ApplySnowStorm(EBlizzardState State, const FVector& InWindDir)
{
	UNiagaraComponent* Snow = SnowNiagaraComponent.Get();
	if (!Snow) return;

	float SpawnRateScale = IdleSnowSpawnRateScale;
	float WindScale = 0.f;

	switch (State)
	{
	case EBlizzardState::Warning:
		SpawnRateScale = WarningSnowSpawnRateScale;
		WindScale = WarningWindVFXScale;
		break;

	case EBlizzardState::Active:
		SpawnRateScale = ActiveSnowSpawnRateScale;
		WindScale = 1.f;
		break;

	case EBlizzardState::Idle:
	default:
		SpawnRateScale = IdleSnowSpawnRateScale;
		WindScale = 0.f;  // Idle 은 바람 없음 -> WindVelocity 가 영벡터가 된다
		break;
	}

	Snow->SetVariableFloat(BlizzardEnvParams::SnowSpawnRateScale, SpawnRateScale);
	Snow->SetVariableVec3(BlizzardEnvParams::WindVelocity, InWindDir * SnowVFXStrength * WindScale);
}

void ABlizzardGimmick::StartBlizzardAmbience()
{
	if (!SnowAmbienceEvent) return;

	// 스위치 컨테이너는 post 시점의 스위치 값으로 초기 레이어가 정해지므로 level1 을 먼저 세팅.
	ApplyAmbienceSwitch(EBlizzardState::Idle);   // BeginPlay 시점 state(Idle) = level1

	// Actor=nullptr 이면 Wwise 글로벌 오브젝트로 나간다 = 위치 감쇠 없이 BGM 처럼 깔린다.
	// (메뉴 BGM 과 같은 경로. TromboneGameInstance.cpp:30)
	AmbiencePlayingID = UAkGameplayStatics::PostEvent(SnowAmbienceEvent, nullptr, 0, FOnAkPostEventCallback());
}

void ABlizzardGimmick::StopBlizzardAmbience()
{
	if (!SnowAmbienceEvent || AmbiencePlayingID == 0) return;

	// 글로벌 오브젝트에 건 소리는 액터가 죽어도 안 꺼진다. 재생 ID 로 직접 정지.
	SnowAmbienceEvent->ExecuteAction(AkActionOnEventType::Stop, nullptr, AmbiencePlayingID);
	AmbiencePlayingID = 0;
}

void ABlizzardGimmick::ApplyAmbienceSwitch(EBlizzardState State)
{
	UAkSwitchValue* Switch = nullptr;
	switch (State)
	{
	case EBlizzardState::Warning:	Switch = AmbienceSwitchLevel2;	break;
	case EBlizzardState::Active:	Switch = AmbienceSwitchLevel3;	break;
	case EBlizzardState::Idle:
	default:						Switch = AmbienceSwitchLevel1;	break;
	}
	if (!Switch) return;

	// 이벤트와 같은 글로벌 오브젝트에 걸어야 한다. 다른 오브젝트에 걸면 레이어 전환이 조용히 무시된다.
	// (Actor=nullptr 이면 글로벌 오브젝트로 간다. UAkGameplayStatics::SetSwitch 는 null 을 거부해서 못 쓴다)
	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		AudioDevice->SetSwitch(Switch, nullptr);
	}
}

void ABlizzardGimmick::SetState(EBlizzardState NewState)
{
	if (!HasAuthority()) return;
	if (BlizzardState == NewState) return;

	BlizzardState = NewState;
	OnRep_BlizzardState();  // 리슨 서버 호스트에서도 연출 이벤트가 발화되도록 수동 호출
	ForceNetUpdate();
}

void ABlizzardGimmick::TickExposure()
{
	if (!HasAuthority() || BlizzardState != EBlizzardState::Active) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 무효 위크포인터 정리
	for (auto It = ExposureTimeMap.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid()) It.RemoveCurrent();
	}

	TArray<ADefaultTromboneCharacter*> Characters;
	for (TActorIterator<ADefaultTromboneCharacter> It(World); It; ++It)
	{
		ADefaultTromboneCharacter* Character = *It;
		if (!IsValid(Character)) continue;
		Characters.Add(Character);
	}

	const FVector WindDir = WindDirection;
	if (WindDir.IsNearlyZero()) return;

	PushTargets.Reset();

	for (ADefaultTromboneCharacter* Character : Characters)
	{
		// 안전지대 안: 밀림/슬로우/누적 없음, 누적 리셋
		if (IsCharacterInShelter(Character))
		{
			ExposureTimeMap.Add(Character, 0.f);
			RemoveSlow(Character);
			continue;
		}

		// 노출 + 래그돌 중: 누적 0 유지(기상 후 새로 카운트), 슬로우 제거, 바람 방향으로 굴리기
		if (Character->IsRagdoll())
		{
			ExposureTimeMap.Add(Character, 0.f);
			RemoveSlow(Character);
			if (bRollRagdolledCharacters)
			{
				if (USkeletalMeshComponent* Mesh = Character->GetMesh())
				{
					Mesh->AddImpulse(
						WindDir * RagdollRollImpulsePerSecond * ExposureCheckInterval,
						TromboneBones::Pelvis,
						true
					);
				}
			}
			continue;
		}

		// 노출 + 일반: 누적 → 임계 도달 시 래그돌
		const float NewExposure = ExposureTimeMap.FindRef(Character) + ExposureCheckInterval;

		if (NewExposure >= RagdollExposureThreshold)
		{
			TriggerRagdoll(Character);

			// 발동 성공(무적/스턴 게이트 통과) 시에만 누적 리셋. 게이트되면 누적 유지 → 해제 직후 재시도
			if (Character->IsRagdoll())
			{
				ExposureTimeMap.Add(Character, 0.f);
				RemoveSlow(Character);
				continue;
			}
		}

		ExposureTimeMap.Add(Character, NewExposure);

		// 밀어내기는 Tick 에서 매 프레임 속도 보충으로 처리 (펄스 임펄스는 Walking 브레이킹에 씹힘)
		PushTargets.Add(Character);

		// 역풍 이동 시 슬로우 (이동 의도 = 가속도 기준, 속도는 밀림으로 오염됨)
		bool bMovingIntoWind = false;
		if (const UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			const FVector Accel = Move->GetCurrentAcceleration();
			if (!Accel.IsNearlyZero())
			{
				const float Dot = FVector::DotProduct(Accel.GetSafeNormal2D(), WindDir);
				const bool bHasSlow = ActiveSlowEffects.Contains(Character);
				// 히스테리시스: 적용/해제 임계를 분리해 방향 전환 시 깜빡임 방지
				if (!bHasSlow && Dot < SlowApplyDotThreshold)
				{
					bMovingIntoWind = true;
				}
				else if (bHasSlow && Dot < SlowReleaseDotThreshold)
				{
					bMovingIntoWind = true;
				}
			}
		}

		if (bMovingIntoWind)
		{
			ApplySlow(Character);
		}
		else
		{
			RemoveSlow(Character);
		}
	}
}

void ABlizzardGimmick::GatherShelters()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	Shelters.Reset();
	for (TActorIterator<ABlizzardShelter> It(World); It; ++It)
	{
		if (ABlizzardShelter* Shelter = *It)
		{
			Shelters.Add(Shelter);
		}
	}
}

void ABlizzardGimmick::OpenRandomShelterDoors()
{
	if (!HasAuthority()) return;

	// 문이 지정된 쉘터만 후보. 문 없는 쉘터를 뽑으면 그 자리가 그냥 날아간다.
	TArray<ABlizzardShelter*> Candidates;
	for (const TWeakObjectPtr<ABlizzardShelter>& ShelterPtr : Shelters)
	{
		ABlizzardShelter* Shelter = ShelterPtr.Get();
		if (Shelter && Shelter->HasDoor())
		{
			Candidates.Add(Shelter);
		}
	}

	// 앞에서 OpenShelterCount 개만 필요하므로 그만큼만 셔플한다 (Fisher-Yates 부분 셔플).
	const int32 OpenCount = FMath::Clamp(OpenShelterCount, 0, Candidates.Num());
	for (int32 i = 0; i < OpenCount; ++i)
	{
		Candidates.Swap(i, FMath::RandRange(i, Candidates.Num() - 1));
	}

	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		Candidates[i]->SetDoorOpen(i < OpenCount);   // 나머지는 닫아 이전 라운드 잔여 상태를 정리
	}
}

void ABlizzardGimmick::CloseAllShelterDoors()
{
	if (!HasAuthority()) return;

	for (const TWeakObjectPtr<ABlizzardShelter>& ShelterPtr : Shelters)
	{
		if (ABlizzardShelter* Shelter = ShelterPtr.Get())
		{
			Shelter->SetDoorOpen(false);
		}
	}
}

void ABlizzardGimmick::GatherPlayerStarts()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	CachedPlayerStarts.Reset();
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (APlayerStart* Start = *It)
		{
			CachedPlayerStarts.Add(Start);
		}
	}
}

void ABlizzardGimmick::EjectCharactersFromShelters()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	CachedPlayerStarts.RemoveAll([](const TWeakObjectPtr<APlayerStart>& Start) { return !Start.IsValid(); });
	if (CachedPlayerStarts.Num() == 0)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("레벨에 PlayerStart 가 없어 쉘터 점거자를 내보내지 못했다"));
		return;
	}

	for (TActorIterator<ADefaultTromboneCharacter> It(World); It; ++It)
	{
		ADefaultTromboneCharacter* Character = *It;
		if (!IsValid(Character)) continue;
		if (!IsCharacterInShelter(Character)) continue;

		TeleportToRandomPlayerStart(Character);
	}
}

void ABlizzardGimmick::TeleportToRandomPlayerStart(ADefaultTromboneCharacter* Character)
{
	if (!Character) return;

	APlayerStart* Start = CachedPlayerStarts[FMath::RandRange(0, CachedPlayerStarts.Num() - 1)].Get();
	if (!Start) return;

	// 래그돌 중엔 액터만 옮겨도 물리 바디가 안 따라온다. 먼저 기상시켜 캡슐/이동모드를 되돌린다.
	// StopRagdoll 이 캡슐을 기상 위치로 스냅시키므로 텔레포트가 반드시 뒤에 와야 한다.
	// (0.2초 블렌드아웃 동안은 이동 복제가 꺼져 있어 원격 클라에선 위치가 조금 늦게 따라온다)
	if (Character->IsRagdoll())
	{
		if (UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent())
		{
			Ragdoll->StopRagdoll();
		}
	}

	const FRotator TargetRotation = Start->GetActorRotation();

	// TeleportPhysics: 블렌드아웃 중이라 아직 시뮬레이션 중인 래그돌 바디까지 같이 끌고 간다.
	Character->SetActorLocationAndRotation(Start->GetActorLocation(), TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// 컨트롤 회전은 클라가 소유하므로 서버에서 대입해봤자 다음 이동 패킷에 덮인다. 클라 RPC 로 밀어줘야 한다.
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		PC->ClientSetRotation(TargetRotation);
	}
}

bool ABlizzardGimmick::IsCharacterInShelter(const ACharacter* Character) const
{
	if (!Character) return false;

	const FVector Loc = Character->GetActorLocation();
	for (const TWeakObjectPtr<ABlizzardShelter>& ShelterPtr : Shelters)
	{
		const ABlizzardShelter* Shelter = ShelterPtr.Get();
		// 문이 닫힌 천막은 안전지대가 아니다. 문이 없는 쉘터는 기존대로 항상 안전.
		if (Shelter && Shelter->IsSheltering() && Shelter->IsLocationInsideShelter(Loc))
		{
			return true;
		}
	}
	return false;
}

void ABlizzardGimmick::ApplySlow(ADefaultTromboneCharacter* Character)
{
	if (!HasAuthority() || !Character || !BlizzardSlowEffectClass) return;

	// 이미 적용 중이면 스킵
	if (ActiveSlowEffects.Contains(Character)) return;

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC) return;

	// 방어적 중복 방지 (핸들이 유실됐지만 GE는 남아있는 경우)
	FGameplayEffectQuery SlowQuery;
	SlowQuery.EffectDefinition = BlizzardSlowEffectClass;
	if (ASC->GetActiveEffects(SlowQuery).Num() > 0) return;

	const UGameplayEffect* GE = BlizzardSlowEffectClass->GetDefaultObject<UGameplayEffect>();
	if (!GE) return;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(GE, 1.f, Context);
	if (Handle.IsValid())
	{
		ActiveSlowEffects.Add(Character, Handle);
	}
}

void ABlizzardGimmick::RemoveSlow(ACharacter* Character)
{
	if (!HasAuthority() || !Character) return;

	FActiveGameplayEffectHandle* FoundHandle = ActiveSlowEffects.Find(Character);
	if (!FoundHandle) return;

	if (const IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Character))
	{
		if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
		{
			if (FoundHandle->IsValid())
			{
				ASC->RemoveActiveGameplayEffect(*FoundHandle);
			}
		}
	}

	ActiveSlowEffects.Remove(Character);
}

void ABlizzardGimmick::RemoveAllSlows()
{
	if (!HasAuthority()) return;

	for (auto& Pair : ActiveSlowEffects)
	{
		ACharacter* Character = Pair.Key.Get();
		if (!Character) continue;

		if (const IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Character))
		{
			if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
			{
				if (Pair.Value.IsValid())
				{
					ASC->RemoveActiveGameplayEffect(Pair.Value);
				}
			}
		}
	}

	ActiveSlowEffects.Empty();
}

void ABlizzardGimmick::TriggerRagdoll(ATromboneCharacterBase* Character)
{
	if (!HasAuthority() || !Character) return;
	if (!Character->Implements<UCombatReceiver>()) return;

	// FHitData 경유: 캐릭터의 무적/스턴/래그돌 게이트를 존중하고, RagdollComponent 직접 접근을 피함
	FHitData HitData;
	HitData.HitDirection = WindDirection;
	HitData.KnockbackForce = 0.f;
	HitData.HitReaction = EHitReactionType::Ragdoll;
	HitData.HitInstigator = EHitInstigatorType::Blizzard;

	ICombatReceiver::Execute_OnHitReceived(Character, HitData);

	// 발동 성공 시 즉시 굴림 임펄스 (StartRagdoll 이 동기적으로 물리를 켠다)
	if (Character->IsRagdoll())
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			Mesh->AddImpulse(WindDirection * RagdollTriggerRollImpulse, TromboneBones::Pelvis, true);
		}
	}
}


#if WITH_EDITOR
void ABlizzardGimmick::EditorForEachEnvPair(TFunctionRef<void(USceneComponent*, USceneComponent*, USceneComponent*, USceneComponent*)> Fn)
{
	// 소프트 참조를 로드해 라이브 컴포넌트를 추출하고, 대응 템플릿들과 함께 Fn 을 호출한다.
	if (ADirectionalLight* Sun = SunLight.LoadSynchronous())
	{
		if (USceneComponent* Live = Sun->GetLightComponent())
		{
			Fn(Live, WarningSunTemplate, ActiveSunTemplate, NormalSunTemplate);
		}
	}
	if (AExponentialHeightFog* Fog = HeightFog.LoadSynchronous())
	{
		if (USceneComponent* Live = Fog->GetComponent())
		{
			Fn(Live, WarningFogTemplate, ActiveFogTemplate, NormalFogTemplate);
		}
	}
	if (ASkyAtmosphere* Atmo = SkyAtmosphere.LoadSynchronous())
	{
		if (USceneComponent* Live = Atmo->GetComponent())
		{
			Fn(Live, WarningAtmosphereTemplate, ActiveAtmosphereTemplate, NormalAtmosphereTemplate);
		}
	}
	if (ASkyLight* Sky = SkyLightActor.LoadSynchronous())
	{
		if (USceneComponent* Live = Sky->GetLightComponent())
		{
			Fn(Live, WarningSkyLightTemplate, ActiveSkyLightTemplate, NormalSkyLightTemplate);
		}
	}
}

namespace
{
	/** 상태에 대응하는 템플릿 고르기. 저장/미리보기가 같은 규칙을 쓰도록 한 곳에 모은다. */
	USceneComponent* PickStateTemplate(EBlizzardState State, USceneComponent* Warn, USceneComponent* Active, USceneComponent* Normal)
	{
		switch (State)
		{
		case EBlizzardState::Warning: return Warn;
		case EBlizzardState::Active:  return Active;
		default:                      return Normal;
		}
	}

	const TCHAR* StateDisplayName(EBlizzardState State)
	{
		switch (State)
		{
		case EBlizzardState::Warning: return TEXT("예고");
		case EBlizzardState::Active:  return TEXT("눈보라");
		default:                      return TEXT("평상시");
		}
	}
}

void ABlizzardGimmick::EditorSaveFromWorld(EBlizzardState State)
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Blizzard] 저장 버튼은 에디터 월드에서만 동작합니다."));
		return;
	}

	FScopedTransaction Tx(NSLOCTEXT("Blizzard", "SaveStateFromWorld", "눈보라 상태 저장 (월드→템플릿)"));
	Modify();

	EditorForEachEnvPair([State](USceneComponent* Live, USceneComponent* Warn, USceneComponent* Active, USceneComponent* Normal)
	{
		USceneComponent* Template = PickStateTemplate(State, Warn, Active, Normal);
		if (!Template) return;
		Template->Modify();
		FBlizzardEnvCopyUtil::CopyProperties(Live, Template);
	});

	// 구름 커버리지(raw) 도 월드 머티리얼에서 캡처. 평상시(NormalCoverage)는 Transient 라
	// 런타임 BeginPlay 에서 다시 잡는다 → 저장 대상이 아니다.
	if (State != EBlizzardState::Idle)
	{
		if (AVolumetricCloud* Cloud = CloudActor.LoadSynchronous())
		{
			if (const UVolumetricCloudComponent* CloudComp = Cloud->FindComponentByClass<UVolumetricCloudComponent>())
			{
				if (UMaterialInterface* Mat = CloudComp->GetMaterial())
				{
					float Cov = 0.f;
					if (Mat->GetScalarParameterValue(FMaterialParameterInfo(BlizzardEnvParams::Coverage), Cov))
					{
						(State == EBlizzardState::Warning ? WarningCoverage : FrozenCoverage) = Cov;
					}
				}
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ABlizzardShelter> It(World); It; ++It)
		{
			if (ABlizzardShelter* Shelter = *It) Shelter->EditorSaveState(State);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Blizzard] %s 상태를 월드에서 템플릿으로 저장했습니다.%s"),
		StateDisplayName(State),
		State == EBlizzardState::Idle ? TEXT(" (구름 커버리지는 평상시 저장 제외 — 재생 시 자동 캡처)") : TEXT(""));
}

void ABlizzardGimmick::EditorLoadToWorld(EBlizzardState State)
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Blizzard] 미리보기 버튼은 에디터 월드에서만 동작합니다."));
		return;
	}

	EditorEnsureNormalBackup();

	FScopedTransaction Tx(NSLOCTEXT("Blizzard", "LoadStateToWorld", "눈보라 상태 미리보기 (템플릿→월드)"));

	// 평상시만 전체 복사다. 평상시 값의 상당수가 클래스 기본값과 같아서(태양색 흰색 등)
	// 바뀐 값만 덮는 방식으로는 전조/눈보라 룩을 되돌릴 수 없다.
	const bool bFullCopy = (State == EBlizzardState::Idle);

	EditorForEachEnvPair([State, bFullCopy](USceneComponent* Live, USceneComponent* Warn, USceneComponent* Active, USceneComponent* Normal)
	{
		USceneComponent* Template = PickStateTemplate(State, Warn, Active, Normal);
		if (!Template) return;
		Live->Modify();
		const bool bChanged = bFullCopy
			? FBlizzardEnvCopyUtil::CopyProperties(Template, Live)
			: FBlizzardEnvCopyUtil::CopyOverriddenProperties(Template, Live);
		if (bChanged)
		{
			Live->MarkRenderStateDirty();
		}
	});

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ABlizzardShelter> It(World); It; ++It)
		{
			if (ABlizzardShelter* Shelter = *It) Shelter->EditorLoadState(State);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Blizzard] %s 상태를 월드에 미리보기했습니다.%s (구름 커버리지는 미리보기 제외 — 재생 시 반영) 되돌리려면 '평상시 복원'."),
		StateDisplayName(State), bFullCopy ? TEXT(" (전체 복사)") : TEXT(""));
}

void ABlizzardGimmick::EditorEnsureNormalBackup()
{
	if (EditorNormalBackups.Num() > 0) return;  // 세션 1회만 캡처

	EditorForEachEnvPair([this](USceneComponent* Live, USceneComponent* /*Warn*/, USceneComponent* /*Active*/, USceneComponent* /*Normal*/)
	{
		EditorNormalBackups.Add(FBlizzardEnvCopyUtil::CreateSnapshot(Live));
		EditorBackupLiveComps.Add(Live);
	});
}

void ABlizzardGimmick::RestoreNormalToWorld()
{
	if (const UWorld* W = GetWorld(); W && W->IsGameWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Blizzard] 복원 버튼은 에디터 월드에서만 동작합니다."));
		return;
	}
	if (EditorNormalBackups.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Blizzard] 복원할 평상시 백업이 없습니다. (미리보기를 먼저 눌러야 백업이 생깁니다)"));
		return;
	}

	FScopedTransaction Tx(NSLOCTEXT("Blizzard", "RestoreNormal", "평상시 복원"));

	for (int32 i = 0; i < EditorNormalBackups.Num(); ++i)
	{
		USceneComponent* Live = EditorBackupLiveComps.IsValidIndex(i) ? EditorBackupLiveComps[i].Get() : nullptr;
		USceneComponent* Backup = EditorNormalBackups[i];
		if (!Live || !Backup) continue;
		Live->Modify();
		FBlizzardEnvCopyUtil::CopyProperties(Backup, Live);
		Live->MarkRenderStateDirty();
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ABlizzardShelter> It(World); It; ++It)
		{
			if (ABlizzardShelter* Shelter = *It) Shelter->EditorRestoreNormal();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Blizzard] 평상시 상태로 복원했습니다."));
}

void ABlizzardGimmick::SaveWarningFromWorld() { EditorSaveFromWorld(EBlizzardState::Warning); }
void ABlizzardGimmick::SaveActiveFromWorld()  { EditorSaveFromWorld(EBlizzardState::Active); }
void ABlizzardGimmick::SaveNormalFromWorld()  { EditorSaveFromWorld(EBlizzardState::Idle); }
void ABlizzardGimmick::LoadWarningToWorld()   { EditorLoadToWorld(EBlizzardState::Warning); }
void ABlizzardGimmick::LoadActiveToWorld()    { EditorLoadToWorld(EBlizzardState::Active); }
void ABlizzardGimmick::LoadNormalToWorld()    { EditorLoadToWorld(EBlizzardState::Idle); }
#endif // WITH_EDITOR
