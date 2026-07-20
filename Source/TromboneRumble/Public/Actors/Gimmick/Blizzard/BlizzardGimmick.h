// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameplayEffectTypes.h"
#include "BlizzardGimmick.generated.h"

class ADirectionalLight;
class UNiagaraComponent;
class UGameplayEffect;
class ACharacter;
class ADefaultTromboneCharacter;
class ATromboneCharacterBase;
class ABlizzardShelter;
class UAkAudioEvent;
class UAkSwitchValue;
class UAkComponent;

UENUM(BlueprintType)
enum class EBlizzardState : uint8
{
	Idle,
	Warning,
	Active,
};

/** 환경 라이팅 한 세트. 상태 전이 시 시작/목표 세트를 잡아두고 BP 타임라인 알파로 보간한다. */
struct FBlizzardEnvValues
{
	float SunIntensity = 0.f;
	FLinearColor SunColor = FLinearColor::White;

	float FogDensity = 0.f;
	FLinearColor FogColor = FLinearColor::White;
	float FogStart = 0.f;

	FLinearColor SkyLuminance = FLinearColor::White;

	FLinearColor SkyLightColor = FLinearColor::White;
	float SkyLightIntensity = 0.f;

	float CloudCoverage = 0.f;
};

/** ABlizzardGimmick
 * 설원맵 전역 눈보라 기믹. 20~30초 랜덤 간격으로 발동한다.
 * Idle -> Warning(예고) -> Active(눈보라) -> Idle 상태머신을 서버에서 구동하고,
 * 상태/바람 방향만 복제해 클라이언트 BP가 VFX/사운드를 재생한다.
 * Active 동안 서버가 주기적으로 각 플레이어의 노출 여부를 판정해
 * 밀어내기 / 역풍 슬로우 / 장시간 노출 시 래그돌 효과를 적용한다.
 */
UCLASS()
class TROMBONERUMBLE_API ABlizzardGimmick : public AGimmickBase
{
	GENERATED_BODY()

public:
	ABlizzardGimmick();

	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** BP FrozenTL 의 Update 에서 호출 (Alpha 0~1). 상태 전이 때 잡아둔 시작->목표 라이팅을 보간 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "Blizzard")
	void UpdateEnvironmentBlend(float Alpha);

protected:
	/** 상태 전이 시 클라/서버 양쪽에서 호출된다. BP 에서 FrozenTL PlayFromStart + 사운드 연출을 구현한다.
	 * 환경 라이팅/눈 VFX/쉘터 라이트는 C++(HandleBlizzardStateChanged)에서 이미 처리하므로 BP 에서 건드리지 말 것.
	 * InWindDirection 은 월드 스페이스. Local Space 나이아가라(회전된 VFX 액터)에 넘길 때는
	 * 해당 컴포넌트 트랜스폼으로 InverseTransformDirection 변환 후 전달할 것 (안 하면 물리 밀림과 VFX 방향이 어긋남). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Blizzard")
	void OnBlizzardStateChanged(EBlizzardState NewState, const FVector& InWindDirection);

	//~ Config
	/** 눈보라 발동 간격 최소 (초) */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "발동 간격 최소"))
	float MinIntervalSeconds = 20.f;

	/** 눈보라 발동 간격 최대 (초) */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "발동 간격 최대"))
	float MaxIntervalSeconds = 30.f;

	/** 예고(Warning) 지속 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "예고 시간"))
	float WarningDuration = 3.f;

	/** 눈보라(Active) 지속 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "지속 시간"))
	float ActiveDuration = 15.f;

	/** 노출 판정 주기 (초) */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "노출 판정 주기"))
	float ExposureCheckInterval = 0.2f;

	/** 이 시간 이상 연속 노출되면 래그돌 (초) */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "래그돌 임계 시간"))
	float RagdollExposureThreshold = 5.f;

	/** 바람 밀림 비율. 바람 세기 = 피해자의 현재 걷기 속도 × 이 값 (플레이어 이동과 합성됨).
	 * 0.6 이면: 정지 시 걷기속도의 60%로 밀리고, 역풍 이동 시 자기 속도의 40%로 전진.
	 * 1.0 이상이면 걷기로는 역풍 전진 불가. 슬로우/버프/악기장착 속도 변화에 자동 비례. */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (ClampMin = "0.0", UIMax = "1.0", DisplayName = "바람 밀림 비율"))
	float WindPushRatio = 0.6f;

	/** 역풍 이동 판정 내적 임계값 (이 값보다 작으면 = 역풍이면 슬로우 적용) */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "슬로우 적용 내적"))
	float SlowApplyDotThreshold = -0.35f;

	/** 슬로우 해제 내적 임계값 (히스테리시스: 적용보다 높게 잡아 깜빡임 방지) */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "슬로우 해제 내적"))
	float SlowReleaseDotThreshold = -0.1f;

	/** 래그돌 발동 직후 굴리는 초기 임펄스 */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "래그돌 초기 굴림 임펄스"))
	float RagdollTriggerRollImpulse = 1200.f;

	/** 래그돌 중 지속 굴리는 초당 임펄스 */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "래그돌 지속 굴림 임펄스"))
	float RagdollRollImpulsePerSecond = 600.f;

	/** 래그돌 상태인 캐릭터를 바람 방향으로 계속 굴릴지 여부 */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "래그돌 굴리기 사용"))
	bool bRollRagdolledCharacters = true;

	/** 바람 축(월드 공간). 발동마다 이 축의 +/- 한 방향이 랜덤 선택된다. */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (DisplayName = "바람 축"))
	FVector WindAxis = FVector(1.f, 0.f, 0.f);

	/** 역풍 이동 시 적용할 슬로우 GameplayEffect (Infinite duration 권장) */
	UPROPERTY(EditDefaultsOnly, Category = "Blizzard|GAS")
	TSubclassOf<UGameplayEffect> BlizzardSlowEffectClass;
	//~ Config

private:
	//~ Env: 레벨에 배치된 라이팅/VFX 액터. 레벨 인스턴스에서 지정한다.
	//  주의: Transient 를 붙이면 레벨 저장 시 참조가 날아간다 (FProperty::ShouldSerializeValue 가 스킵).
	UPROPERTY(EditInstanceOnly, Category = "Blizzard|Env", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<class ANiagaraActor> SnowVFXActor;

	UPROPERTY(EditInstanceOnly, Category = "Blizzard|Env", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<class ADirectionalLight> SunLight;

	UPROPERTY(EditInstanceOnly, Category = "Blizzard|Env", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<class AExponentialHeightFog> HeightFog;

	UPROPERTY(EditInstanceOnly, Category = "Blizzard|Env", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<class ASkyAtmosphere> SkyAtmosphere;

	UPROPERTY(EditInstanceOnly, Category = "Blizzard|Env", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<class ASkyLight> SkyLightActor;

	UPROPERTY(EditInstanceOnly, Category = "Blizzard|Env", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<class AVolumetricCloud> CloudActor;
	//~

	//~ 위 액터들에서 BeginPlay 에 추출한 컴포넌트. 월드가 소유한 것이라 비소유 약참조로 잡는다.
	TWeakObjectPtr<class UNiagaraComponent> SnowNiagaraComponent;
	TWeakObjectPtr<class UDirectionalLightComponent> SunComponent;
	TWeakObjectPtr<class UExponentialHeightFogComponent> FogComponent;
	TWeakObjectPtr<class USkyAtmosphereComponent> AtmosphereComponent;
	TWeakObjectPtr<class USkyLightComponent> SkyLightComponent;

	/** 구름 커버리지 제어용. 이건 우리가 만든 MID 라 GC 로부터 지켜야 해서 강참조. */
	UPROPERTY(Transient)
	TObjectPtr<class UMaterialInstanceDynamic> CloudMID;
	//~

	//~ 상태별 라이팅 값.
	//  Normal 은 BeginPlay 에 레벨 저작값을 캡처한 것 (Idle 복귀 목표), Warning/Frozen 은 에디터에서 직접 지정한다.
	// 태양광 밝기 (Directional Light 는 lux). Normal 저작값 스케일에 맞춰 조정할 것 — 기존 BP 가 0.02 스케일이었음.
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Blizzard|Env|Sun", meta = (AllowPrivateAccess = "true"))
	float NormalSunIntensity = 0.f;
	// 저녁 노을: 해가 낮아 살짝 어둑하지만 색이 강하게 남음
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sun", meta = (AllowPrivateAccess = "true"))
	float WarningSunIntensity = 2.5f;
	// 눈보라: 두꺼운 구름에 가려 거의 꺼짐
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sun", meta = (AllowPrivateAccess = "true"))
	float FrozenSunIntensity = 0.3f;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Blizzard|Env|Sun", meta = (AllowPrivateAccess = "true"))
	FLinearColor NormalSunColor = FLinearColor::White;
	// 따뜻한 주황 노을빛
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sun", meta = (AllowPrivateAccess = "true"))
	FLinearColor WarningSunColor = FLinearColor(1.0f, 0.55f, 0.28f, 1.f);
	// 차가운 청회색
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sun", meta = (AllowPrivateAccess = "true"))
	FLinearColor FrozenSunColor = FLinearColor(0.55f, 0.68f, 0.9f, 1.f);

	// 안개 짙기. ExponentialHeightFog 의 유일한 짙기 knob 인 FogDensity 에 그대로 적용된다 (기본 저작값 ~0.02).
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Blizzard|Env|Fog", meta = (AllowPrivateAccess = "true"))
	float NormalFogDensity = 0.f;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Fog", meta = (AllowPrivateAccess = "true"))
	float WarningFogDensity = 0.05f;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Fog", meta = (AllowPrivateAccess = "true"))
	float FrozenFogDensity = 0.3f;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Blizzard|Env|Fog", meta = (AllowPrivateAccess = "true"))
	FLinearColor NormalFogColor = FLinearColor::White;
	// 노을빛 따뜻한 안개
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Fog", meta = (AllowPrivateAccess = "true"))
	FLinearColor WarningFogColor = FLinearColor(0.95f, 0.5f, 0.35f, 1.f);
	// 차갑고 어두운 눈보라 안개
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Fog", meta = (AllowPrivateAccess = "true"))
	FLinearColor FrozenFogColor = FLinearColor(0.07f, 0.09f, 0.14f, 1.f);

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Blizzard|Env|Fog", meta = (AllowPrivateAccess = "true"))
	float NormalFogStart = 0.f;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Fog", meta = (AllowPrivateAccess = "true"))
	float WarningFogStart = 0.f;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Fog", meta = (AllowPrivateAccess = "true"))
	float FrozenFogStart = 0.f;

	// SkyAtmosphere SkyLuminanceFactor: 하늘 자체의 색조. 노을은 주황빛, 눈보라는 어두운 청색.
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Blizzard|Env|Sky", meta = (AllowPrivateAccess = "true"))
	FLinearColor NormalSkyLum = FLinearColor::White;
	// 노을빛으로 물든 하늘
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sky", meta = (AllowPrivateAccess = "true"))
	FLinearColor WarningSkyLum = FLinearColor(1.0f, 0.65f, 0.45f, 1.f);
	// 어둡고 차가운 하늘
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sky", meta = (AllowPrivateAccess = "true"))
	FLinearColor FrozenSkyLum = FLinearColor(0.12f, 0.16f, 0.26f, 1.f);

	// SkyLight 색: 그림자/음영에 들어가는 환경광 색조
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Blizzard|Env|Sky", meta = (AllowPrivateAccess = "true"))
	FLinearColor NormalSkyLightColor = FLinearColor::White;
	// 따뜻한 노을 환경광
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sky", meta = (AllowPrivateAccess = "true"))
	FLinearColor WarningSkyLightColor = FLinearColor(1.0f, 0.72f, 0.55f, 1.f);
	// 차가운 눈보라 환경광
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sky", meta = (AllowPrivateAccess = "true"))
	FLinearColor FrozenSkyLightColor = FLinearColor(0.6f, 0.72f, 0.92f, 1.f);

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Blizzard|Env|Sky", meta = (AllowPrivateAccess = "true"))
	float NormalSkyLightIntensity = 0.f;
	// 노을: 아직 밝아서 음영이 완전히 죽지 않게
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sky", meta = (AllowPrivateAccess = "true"))
	float WarningSkyLightIntensity = 0.8f;
	// 눈보라: 전반적으로 어둡게
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Sky", meta = (AllowPrivateAccess = "true"))
	float FrozenSkyLightIntensity = 0.15f;

	// 볼류메트릭 클라우드 커버리지 (머티리얼 "Coverage" 스칼라). 노을엔 살짝, 눈보라엔 잔뜩 덮는다.
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Blizzard|Env|Cloud", meta = (AllowPrivateAccess = "true"))
	float NormalCoverage = 0.f;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Cloud", meta = (AllowPrivateAccess = "true"))
	float WarningCoverage = 0.35f;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Cloud", meta = (AllowPrivateAccess = "true"))
	float FrozenCoverage = 0.85f;
	//~

	//~ 눈 VFX (나이아가라 사용자 파라미터로 전달)
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Snow", meta = (AllowPrivateAccess = "true", DisplayName = "바람 VFX 세기"))
	float SnowVFXStrength = 300.f;

	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Snow", meta = (AllowPrivateAccess = "true", DisplayName = "평상시 눈 스폰 배율"))
	float IdleSnowSpawnRateScale = 1.f;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Snow", meta = (AllowPrivateAccess = "true", DisplayName = "예고 눈 스폰 배율"))
	float WarningSnowSpawnRateScale = 3.f;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Snow", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 눈 스폰 배율"))
	float ActiveSnowSpawnRateScale = 10.f;

	/** 예고 단계 바람 세기 = SnowVFXStrength × 이 값 (눈보라 단계는 1.0 고정) */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Env|Snow", meta = (AllowPrivateAccess = "true", DisplayName = "예고 바람 세기 배율"))
	float WarningWindVFXScale = 0.5f;
	//~

	//~ Audio (Wwise 눈보라 앰비언스).
	//  이벤트를 BeginPlay 에 AmbienceAkComponent 로 1회 post 하고, 상태 전이마다 스위치만 바꿔 레이어를 전환한다 (크로스페이드는 Wwise 저작).
	//  BlizzardGimmick 은 C++ 루트가 없어 여기서 네이티브 루트를 만들고 Ak 컴포넌트를 붙인다.
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkComponent> AmbienceAkComponent;

	UPROPERTY(EditAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 앰비언스 이벤트"))
	TObjectPtr<UAkAudioEvent> SnowAmbienceEvent;

	// level1 = 평상시(Idle), level2 = 예고(Warning), level3 = 폭풍(Active)
	UPROPERTY(EditAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true", DisplayName = "앰비언스 스위치 Lv1 (평상시)"))
	TObjectPtr<UAkSwitchValue> AmbienceSwitchLevel1;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true", DisplayName = "앰비언스 스위치 Lv2 (예고)"))
	TObjectPtr<UAkSwitchValue> AmbienceSwitchLevel2;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true", DisplayName = "앰비언스 스위치 Lv3 (폭풍)"))
	TObjectPtr<UAkSwitchValue> AmbienceSwitchLevel3;
	//~

	/** 연출용 쉘터 목록 (BeginPlay 1회 수집). 게임플레이 판정용 Shelters 와 별개 — 이쪽은 클라에서도 필요하다. */
	TArray<TWeakObjectPtr<class ABlizzardShelter>> CachedBlizzardShelters;

	//~ Env 연출 (데디케이티드 서버 제외)
	void InitializeBlizzardComponents();
	void InitializeBlizzardProperties();
	void HandleBlizzardStateChanged(EBlizzardState NewState, const FVector& InWindDir);
	FBlizzardEnvValues CaptureCurrentEnvValues() const;
	FBlizzardEnvValues GetStateTargetValues(EBlizzardState State) const;
	void ApplyEnvValues(const FBlizzardEnvValues& Values);
	void ApplySnowStorm(EBlizzardState State, const FVector& InWindDir);
	void StartBlizzardAmbience();               // BeginPlay: level1 세팅 후 이벤트 1회 post
	void ApplyAmbienceSwitch(EBlizzardState State);  // 상태별 앰비언스 스위치 전환

	/** 현재 진행 중인 라이팅 페이드 구간. UpdateEnvironmentBlend 가 이 사이를 보간한다. */
	FBlizzardEnvValues BlendStart;
	FBlizzardEnvValues BlendTarget;
	//~

	//~ Replicated
	UPROPERTY(ReplicatedUsing = OnRep_BlizzardState)
	EBlizzardState BlizzardState = EBlizzardState::Idle;

	UPROPERTY(Replicated)
	FVector_NetQuantizeNormal WindDirection = FVector::ZeroVector;

	UFUNCTION()
	void OnRep_BlizzardState();
	//~ Replicated

	//~ Server-only state machine
	void SetState(EBlizzardState NewState);
	void ScheduleNextBlizzard();
	void StartWarning();
	void StartBlizzard();
	void EndBlizzard();
	void TickExposure();
	//~

	//~ Server-only effect helpers
	void GatherShelters();
	bool IsCharacterInShelter(const ACharacter* Character) const;
	void ApplySlow(ADefaultTromboneCharacter* Character);
	void RemoveSlow(ACharacter* Character);
	void RemoveAllSlows();
	void TriggerRagdoll(ATromboneCharacterBase* Character);
	//~

	FTimerHandle ScheduleTimerHandle;
	FTimerHandle PhaseTimerHandle;
	FTimerHandle ExposureTimerHandle;

	/** 서버 전용: 캐릭터별 연속 노출 누적 시간 */
	TMap<TWeakObjectPtr<ACharacter>, float> ExposureTimeMap;

	/** 서버 전용: 프레임 단위 밀기(Tick) 대상. TickExposure 마다 재구성 (노출 + 비래그돌만) */
	TArray<TWeakObjectPtr<ADefaultTromboneCharacter>> PushTargets;

	/** 서버 전용: 각 캐릭터에게 건 슬로우 GE 핸들 */
	UPROPERTY()
	TMap<TWeakObjectPtr<ACharacter>, FActiveGameplayEffectHandle> ActiveSlowEffects;

	/** 서버 전용: 수집한 안전지대 목록 (StartBlizzard 마다 갱신) */
	TArray<TWeakObjectPtr<ABlizzardShelter>> Shelters;
};
