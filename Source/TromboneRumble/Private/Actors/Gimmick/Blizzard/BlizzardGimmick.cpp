// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/Blizzard/BlizzardGimmick.h"
#include "Actors/Gimmick/Blizzard/BlizzardShelter.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/TromboneCharacterBase.h"
#include "Interfaces/CombatReceiver.h"
#include "Utilities/Defines.h"
#include "GameFramework/CharacterMovementComponent.h"
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
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "AkSwitchValue.h"
#include "Net/UnrealNetwork.h"

namespace BlizzardEnvParams
{
	// 눈 나이아가라 시스템의 사용자 노출 파라미터
	static const FName SnowSpawnRateScale(TEXT("SnowSpawnRateScale"));
	static const FName WindVelocity(TEXT("WindVelocity"));

	// 볼류메트릭 클라우드 머티리얼의 스칼라 파라미터
	static const FName Coverage(TEXT("Coverage"));
}

namespace
{
	FBlizzardEnvValues LerpEnvValues(const FBlizzardEnvValues& A, const FBlizzardEnvValues& B, float Alpha)
	{
		FBlizzardEnvValues Result;

		Result.SunIntensity		= FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
		Result.SunColor			= FMath::Lerp(A.SunColor, B.SunColor, Alpha);
		Result.FogDensity		= FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
		Result.FogColor			= FMath::Lerp(A.FogColor, B.FogColor, Alpha);
		Result.FogStart			= FMath::Lerp(A.FogStart, B.FogStart, Alpha);
		Result.SkyLuminance		= FMath::Lerp(A.SkyLuminance, B.SkyLuminance, Alpha);
		Result.SkyLightColor	= FMath::Lerp(A.SkyLightColor, B.SkyLightColor, Alpha);
		Result.SkyLightIntensity= FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
		Result.CloudCoverage	= FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);

		return Result;
	}
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

	// 눈보라 앰비언스를 담을 네이티브 루트 + Ak 컴포넌트 (SpotlightZone 패턴).
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	AmbienceAkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AmbienceAkComponent"));
	if (AmbienceAkComponent)
	{
		AmbienceAkComponent->OcclusionRefreshInterval = 0.f;
		AmbienceAkComponent->SetupAttachment(SceneRoot);
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
		SetState(EBlizzardState::Idle);
	}

	// Super가 bIsActive=false + ClearAllTimersForObject 로 3개 타이머를 정리한다.
	Super::Deactivate();
}

void ABlizzardGimmick::BeginPlay()
{
	Super::BeginPlay();

	// 라이팅/VFX 는 순수 연출이라 데디케이티드 서버에는 불필요.
	if (GetNetMode() == NM_DedicatedServer) return;

	InitializeBlizzardComponents();
	InitializeBlizzardProperties();
	StartBlizzardAmbience();
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
	GatherShelters();

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

void ABlizzardGimmick::InitializeBlizzardProperties()
{
	// 레벨에 저작된 평상시 라이팅을 Normal 세트로 캡처한다 (= Idle 복귀 목표값).
	if (const UDirectionalLightComponent* Sun = SunComponent.Get())
	{
		NormalSunIntensity = Sun->Intensity;
		NormalSunColor = Sun->GetLightColor();
	}

	if (const UExponentialHeightFogComponent* Fog = FogComponent.Get())
	{
		NormalFogDensity = Fog->FogDensity;
		NormalFogStart = Fog->StartDistance;
		NormalFogColor = Fog->FogInscatteringLuminance;
	}

	if (const USkyAtmosphereComponent* Atmo = AtmosphereComponent.Get())
	{
		NormalSkyLum = Atmo->SkyLuminanceFactor;
	}

	if (const USkyLightComponent* Sky = SkyLightComponent.Get())
	{
		NormalSkyLightColor = Sky->GetLightColor();
		NormalSkyLightIntensity = Sky->Intensity;
	}

	if (CloudMID)
	{
		NormalCoverage = CloudMID->K2_GetScalarParameterValue(BlizzardEnvParams::Coverage);
	}
}

void ABlizzardGimmick::OnRep_BlizzardState()
{
	HandleBlizzardStateChanged(BlizzardState, WindDirection);
}

void ABlizzardGimmick::HandleBlizzardStateChanged(EBlizzardState NewState, const FVector& InWindDir)
{
	if (GetNetMode() == NM_DedicatedServer) return;

	// 이전 페이드가 끝나기 전에 상태가 또 바뀔 수 있으므로, 이전 목표가 아니라 "현재 실값"에서 이어 간다.
	// Active -> Idle 은 Warning 값을 거치지 않고 곧장 Normal 로 돌아간다.
	BlendStart = CaptureCurrentEnvValues();
	BlendTarget = GetStateTargetValues(NewState);

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

FBlizzardEnvValues ABlizzardGimmick::CaptureCurrentEnvValues() const
{
	// 참조가 비어있는 항목은 Normal 값으로 폴백 (보간해도 값이 튀지 않게)
	FBlizzardEnvValues Values = GetStateTargetValues(EBlizzardState::Idle);

	if (const UDirectionalLightComponent* Sun = SunComponent.Get())
	{
		Values.SunIntensity = Sun->Intensity;
		Values.SunColor = Sun->GetLightColor();
	}

	if (const UExponentialHeightFogComponent* Fog = FogComponent.Get())
	{
		Values.FogDensity = Fog->FogDensity;
		Values.FogStart = Fog->StartDistance;
		Values.FogColor = Fog->FogInscatteringLuminance;
	}

	if (const USkyAtmosphereComponent* Atmo = AtmosphereComponent.Get())
	{
		Values.SkyLuminance = Atmo->SkyLuminanceFactor;
	}

	if (const USkyLightComponent* Sky = SkyLightComponent.Get())
	{
		Values.SkyLightColor = Sky->GetLightColor();
		Values.SkyLightIntensity = Sky->Intensity;
	}

	if (CloudMID)
	{
		Values.CloudCoverage = CloudMID->K2_GetScalarParameterValue(BlizzardEnvParams::Coverage);
	}

	return Values;
}

FBlizzardEnvValues ABlizzardGimmick::GetStateTargetValues(EBlizzardState State) const
{
	FBlizzardEnvValues Values;

	switch (State)
	{
	case EBlizzardState::Warning:
		Values.SunIntensity			= WarningSunIntensity;
		Values.SunColor				= WarningSunColor;
		Values.FogDensity			= WarningFogDensity;
		Values.FogColor				= WarningFogColor;
		Values.FogStart				= WarningFogStart;
		Values.SkyLuminance			= WarningSkyLum;
		Values.SkyLightColor		= WarningSkyLightColor;
		Values.SkyLightIntensity	= WarningSkyLightIntensity;
		Values.CloudCoverage		= WarningCoverage;
		break;

	case EBlizzardState::Active:
		Values.SunIntensity			= FrozenSunIntensity;
		Values.SunColor				= FrozenSunColor;
		Values.FogDensity			= FrozenFogDensity;
		Values.FogColor				= FrozenFogColor;
		Values.FogStart				= FrozenFogStart;
		Values.SkyLuminance			= FrozenSkyLum;
		Values.SkyLightColor		= FrozenSkyLightColor;
		Values.SkyLightIntensity	= FrozenSkyLightIntensity;
		Values.CloudCoverage		= FrozenCoverage;
		break;

	case EBlizzardState::Idle:
	default:
		// Normal 세트는 BeginPlay 에 캡처한 레벨 저작값
		Values.SunIntensity			= NormalSunIntensity;
		Values.SunColor				= NormalSunColor;
		Values.FogDensity			= NormalFogDensity;
		Values.FogColor				= NormalFogColor;
		Values.FogStart				= NormalFogStart;
		Values.SkyLuminance			= NormalSkyLum;
		Values.SkyLightColor		= NormalSkyLightColor;
		Values.SkyLightIntensity	= NormalSkyLightIntensity;
		Values.CloudCoverage		= NormalCoverage;
		break;
	}

	return Values;
}

void ABlizzardGimmick::ApplyEnvValues(const FBlizzardEnvValues& Values)
{
	// 주의: 라이트 Mobility 가 Static 이면 setter 가 조용히 무시된다 (AreDynamicDataChangesAllowed).
	if (UDirectionalLightComponent* Sun = SunComponent.Get())
	{
		Sun->SetIntensity(Values.SunIntensity);
		Sun->SetLightColor(Values.SunColor);
	}

	if (UExponentialHeightFogComponent* Fog = FogComponent.Get())
	{
		Fog->SetFogDensity(Values.FogDensity);
		Fog->SetFogInscatteringColor(Values.FogColor);
		Fog->SetStartDistance(Values.FogStart);
	}

	if (USkyAtmosphereComponent* Atmo = AtmosphereComponent.Get())
	{
		Atmo->SetSkyLuminanceFactor(Values.SkyLuminance);
	}

	// 색/강도만 바꾸는 건 RecaptureSky 없이 즉시 반영된다 (큐브맵 재캡처는 하지 말 것 — 히칭).
	if (USkyLightComponent* Sky = SkyLightComponent.Get())
	{
		Sky->SetLightColor(Values.SkyLightColor);
		Sky->SetIntensity(Values.SkyLightIntensity);
	}

	if (CloudMID)
	{
		CloudMID->SetScalarParameterValue(BlizzardEnvParams::Coverage, Values.CloudCoverage);
	}
}

void ABlizzardGimmick::UpdateEnvironmentBlend(float Alpha)
{
	ApplyEnvValues(LerpEnvValues(BlendStart, BlendTarget, Alpha));
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
	if (!SnowAmbienceEvent || !AmbienceAkComponent) return;

	// 스위치 컨테이너는 post 시점의 스위치 값으로 초기 레이어가 정해지므로 level1 을 먼저 세팅.
	ApplyAmbienceSwitch(EBlizzardState::Idle);   // BeginPlay 시점 state(Idle) = level1
	AmbienceAkComponent->PostAkEvent(SnowAmbienceEvent, 0, FOnAkPostEventCallback());
}

void ABlizzardGimmick::ApplyAmbienceSwitch(EBlizzardState State)
{
	if (!AmbienceAkComponent) return;

	UAkSwitchValue* Switch = nullptr;
	switch (State)
	{
	case EBlizzardState::Warning:	Switch = AmbienceSwitchLevel2;	break;
	case EBlizzardState::Active:	Switch = AmbienceSwitchLevel3;	break;
	case EBlizzardState::Idle:
	default:						Switch = AmbienceSwitchLevel1;	break;
	}
	if (!Switch) return;

	// group/state 는 UAkSwitchValue 애셋이 내장하므로 빈 값으로 넘긴다. (CharacterAnimInstance.cpp:129 패턴)
	AmbienceAkComponent->SetSwitch(Switch, FString(TEXT("")), FString(TEXT("")));
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

bool ABlizzardGimmick::IsCharacterInShelter(const ACharacter* Character) const
{
	if (!Character) return false;

	const FVector Loc = Character->GetActorLocation();
	for (const TWeakObjectPtr<ABlizzardShelter>& ShelterPtr : Shelters)
	{
		const ABlizzardShelter* Shelter = ShelterPtr.Get();
		if (Shelter && Shelter->IsLocationInsideShelter(Loc))
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
