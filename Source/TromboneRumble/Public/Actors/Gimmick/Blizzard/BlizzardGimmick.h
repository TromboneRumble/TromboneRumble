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
class APlayerStart;
class UAkAudioEvent;
class UAkSwitchValue;
class USceneComponent;
class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;

UENUM(BlueprintType)
enum class EBlizzardState : uint8
{
	Idle,
	Warning,
	Active,
};

/** 하나의 구동 대상 환경 컴포넌트에 대한 런타임 보간 상태.
 * 라이브 월드 컴포넌트 + 값 스냅샷들(평상시/블렌드 시작/상태별 목표)을 묶는다.
 * 상태별 목표(ResolvedWarning/Active)는 평상시 값 위에 템플릿의 "오버라이드된(=클래스 기본값과 다른)"
 * 프로퍼티만 얹어 만든다 → 아티스트가 템플릿에서 바꾼 값만 구동되고 나머지는 평상시 그대로 유지된다. */
USTRUCT()
struct FBlizzardDrivenEnv
{
	GENERATED_BODY()

	/** 구동 대상 라이브 월드 컴포넌트 (월드 소유라 약참조). */
	TWeakObjectPtr<USceneComponent> Live;

	/** 블렌드 시작 시점의 라이브 값 스냅샷 (전이마다 재캡처). */
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> StartSnapshot = nullptr;

	/** 레벨 저작 평상시 값 = Idle 복귀 목표. */
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> NormalSnapshot = nullptr;

	/** 평상시 + 전조 템플릿 오버라이드 = Warning 목표. */
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> ResolvedWarning = nullptr;

	/** 평상시 + 눈보라 템플릿 오버라이드 = Active 목표. */
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> ResolvedActive = nullptr;

	/** 이 라이브 컴포넌트에 값을 저작하는 전조/눈보라 템플릿 (액터 소유). */
	TWeakObjectPtr<USceneComponent> WarningTemplate;
	TWeakObjectPtr<USceneComponent> ActiveTemplate;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	/** 전조 때 문이 열릴 천막 수. 문이 지정된 쉘터가 이보다 적으면 전부 열린다. */
	UPROPERTY(EditAnywhere, Category = "Blizzard|Config", meta = (ClampMin = "0", DisplayName = "문 열릴 천막 수"))
	int32 OpenShelterCount = 2;

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

	//~ 상태별 라이팅 템플릿 컴포넌트 (읽기 전용 데이터 컨테이너).
	//  각 템플릿에서 "클래스 기본값과 다른" 프로퍼티(밝기/색/안개짙기/하늘색조 등 무엇이든)만
	//  해당 상태에서 라이브 월드 컴포넌트로 구동된다 (BuildDrivenEnvEntries → CopyOverriddenProperties).
	//  아트가 새 값을 조정해도 C++ 수정이 필요 없다.
	//  값을 넣는 경로는 저장 버튼 하나뿐: 레벨의 실제 라이팅 액터를 조정 → "전조/눈보라/평상시 상태 저장" 클릭.
	//  invisible + bAffectsWorld=false 로 렌더/캡처에 관여하지 않고,
	//  bEditableWhenInherited=false 로 Details 직접 편집이 잠겨 있다 (BlizzardGimmick.cpp 생성자 참조).
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "전조 태양"))
	TObjectPtr<UDirectionalLightComponent> WarningSunTemplate;
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 태양"))
	TObjectPtr<UDirectionalLightComponent> ActiveSunTemplate;

	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "전조 안개"))
	TObjectPtr<UExponentialHeightFogComponent> WarningFogTemplate;
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 안개"))
	TObjectPtr<UExponentialHeightFogComponent> ActiveFogTemplate;

	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "전조 대기"))
	TObjectPtr<USkyAtmosphereComponent> WarningAtmosphereTemplate;
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 대기"))
	TObjectPtr<USkyAtmosphereComponent> ActiveAtmosphereTemplate;

	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "전조 스카이라이트"))
	TObjectPtr<USkyLightComponent> WarningSkyLightTemplate;
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 스카이라이트"))
	TObjectPtr<USkyLightComponent> ActiveSkyLightTemplate;

	//~ 평상시 템플릿. 위 둘과 달리 "바뀐 값만" 이 아니라 라이브 상태를 통째로 담는다.
	//  평상시 룩의 상당수가 엔진 기본값과 같아서(태양색 흰색, 안개 인스캐터링 검정 등) 덮어쓰기
	//  방식으로는 전조/눈보라를 되돌릴 수 없기 때문. 그래서 미리보기가 전체 복사다 (EditorLoadToWorld).
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "평상시 태양"))
	TObjectPtr<UDirectionalLightComponent> NormalSunTemplate;
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "평상시 안개"))
	TObjectPtr<UExponentialHeightFogComponent> NormalFogTemplate;
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "평상시 대기"))
	TObjectPtr<USkyAtmosphereComponent> NormalAtmosphereTemplate;
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Env|Templates", meta = (AllowPrivateAccess = "true", DisplayName = "평상시 스카이라이트"))
	TObjectPtr<USkyLightComponent> NormalSkyLightTemplate;
	//~

	// 볼류메트릭 클라우드 커버리지 (머티리얼 "Coverage" 스칼라). 컴포넌트 프로퍼티가 아니라 raw 값 유지.
	// 노을엔 살짝, 눈보라엔 잔뜩 덮는다.
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
	//  이벤트를 BeginPlay 에 Wwise 글로벌 오브젝트로 1회 post 하고, 상태 전이마다 스위치만 바꿔 레이어를 전환한다 (크로스페이드는 Wwise 저작).
	//  BGM 과 같은 경로라 액터 위치 기준 감쇠가 없다. 스위치도 같은 글로벌 오브젝트에 걸어야 한다.
	//  BlizzardGimmick 은 C++ 루트가 없어 여기서 네이티브 루트를 만든다.
	UPROPERTY(VisibleAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 앰비언스 이벤트"))
	TObjectPtr<UAkAudioEvent> SnowAmbienceEvent;

	// level1 = 평상시(Idle), level2 = 예고(Warning), level3 = 폭풍(Active)
	UPROPERTY(EditAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true", DisplayName = "앰비언스 스위치 Lv1 (평상시)"))
	TObjectPtr<UAkSwitchValue> AmbienceSwitchLevel1;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true", DisplayName = "앰비언스 스위치 Lv2 (예고)"))
	TObjectPtr<UAkSwitchValue> AmbienceSwitchLevel2;
	UPROPERTY(EditAnywhere, Category = "Blizzard|Audio", meta = (AllowPrivateAccess = "true", DisplayName = "앰비언스 스위치 Lv3 (폭풍)"))
	TObjectPtr<UAkSwitchValue> AmbienceSwitchLevel3;

	/** post 로 받은 재생 ID. 글로벌 오브젝트에 건 소리라 액터가 죽어도 자동으로 안 꺼져서, 이 ID 로 직접 정지한다. */
	int32 AmbiencePlayingID = 0;
	//~

	/** 연출용 쉘터 목록 (BeginPlay 1회 수집). 게임플레이 판정용 Shelters 와 별개 — 이쪽은 클라에서도 필요하다. */
	TArray<TWeakObjectPtr<class ABlizzardShelter>> CachedBlizzardShelters;

	//~ Env 연출 (데디케이티드 서버 제외)
	void InitializeBlizzardComponents();
	/** BeginPlay: 라이브 컴포넌트별로 평상시/상태별 목표 스냅샷을 구성한다. */
	void BuildDrivenEnvEntries();
	/** 라이브 컴포넌트 + 전조/눈보라 템플릿 한 쌍으로 구동 엔트리 하나를 만든다. */
	void AddDrivenEntry(USceneComponent* Live, USceneComponent* WarnTemplate, USceneComponent* ActiveTemplate);
	void HandleBlizzardStateChanged(EBlizzardState NewState, const FVector& InWindDir);
	/** 전이 시 각 엔트리의 시작 스냅샷을 재캡처하고 비보간(bool/enum) 값을 목표로 스냅한다. */
	void BeginEnvBlend(EBlizzardState NewState);
	/** 엔트리의 상태별 목표 스냅샷을 돌려준다 (Idle=Normal, Warning/Active=Resolved). */
	USceneComponent* GetEnvTargetFor(const FBlizzardDrivenEnv& Entry, EBlizzardState State) const;
	/** 상태별 구름 커버리지 목표값 (raw). */
	float GetStateCoverage(EBlizzardState State) const;
	void ApplySnowStorm(EBlizzardState State, const FVector& InWindDir);
	void StartBlizzardAmbience();               // BeginPlay: level1 세팅 후 이벤트 1회 post
	void StopBlizzardAmbience();                // EndPlay: 재생 ID 로 정지 (안 하면 결과씬/메뉴까지 따라간다)
	void ApplyAmbienceSwitch(EBlizzardState State);  // 상태별 앰비언스 스위치 전환

	/** 구동 대상 환경 컴포넌트별 보간 상태 (태양/안개/대기/스카이라이트). BeginPlay 에 구성. */
	UPROPERTY(Transient)
	TArray<FBlizzardDrivenEnv> DrivenEnvComponents;

	/** 현재 블렌드가 향하는 상태. UpdateEnvironmentBlend 가 시작 스냅샷 → 이 상태 목표로 보간한다. */
	EBlizzardState BlendTargetState = EBlizzardState::Idle;
	/** 블렌드 시작 시점의 구름 커버리지 (raw 보간용). */
	float BlendStartCoverage = 0.f;
	//~

#if WITH_EDITOR
public:
	//~ 에디터 저작 버튼. 아트가 월드에서 라이팅을 직접 만지며 상태를 저장/미리보기 한다.
	//  워크플로우: 미리보기 → 월드에서 튜닝 → 저장 → 평상시 복원. (미리보기 중 레벨 저장 금지, 복원 먼저)
	//  평상시 저장은 미리보기가 걸린 채로 누르지 말 것 — 전조 룩이 평상시로 저장된다. 복원 먼저.
	//  패널 순서는 선언 순서가 아니라 DisplayPriority 로 정해진다 (없으면 함수명 알파벳순).
	UFUNCTION(CallInEditor, Category = "Blizzard|Editor|Save|Load", meta = (DisplayName = "전조 상태 저장", DisplayPriority = "1"))
	void SaveWarningFromWorld();
	UFUNCTION(CallInEditor, Category = "Blizzard|Editor|Save|Load", meta = (DisplayName = "눈보라 상태 저장", DisplayPriority = "2"))
	void SaveActiveFromWorld();
	UFUNCTION(CallInEditor, Category = "Blizzard|Editor|Save|Load", meta = (DisplayName = "평상시 상태 저장", DisplayPriority = "3"))
	void SaveNormalFromWorld();
	UFUNCTION(CallInEditor, Category = "Blizzard|Editor|Save|Load", meta = (DisplayName = "전조 상태 미리보기", DisplayPriority = "4"))
	void LoadWarningToWorld();
	UFUNCTION(CallInEditor, Category = "Blizzard|Editor|Save|Load", meta = (DisplayName = "눈보라 상태 미리보기", DisplayPriority = "5"))
	void LoadActiveToWorld();
	UFUNCTION(CallInEditor, Category = "Blizzard|Editor|Save|Load", meta = (DisplayName = "평상시 상태 미리보기", DisplayPriority = "6"))
	void LoadNormalToWorld();
	UFUNCTION(CallInEditor, Category = "Blizzard|Editor|Save|Load", meta = (DisplayName = "평상시 복원 (미리보기 취소)", DisplayPriority = "7"))
	void RestoreNormalToWorld();

private:
	/** 소프트 참조를 LoadSynchronous 하고 (라이브, 전조, 눈보라, 평상시 템플릿) 조합마다 Fn 을 호출. */
	void EditorForEachEnvPair(TFunctionRef<void(USceneComponent* /*Live*/, USceneComponent* /*Warn*/, USceneComponent* /*Active*/, USceneComponent* /*Normal*/)> Fn);
	void EditorSaveFromWorld(EBlizzardState State);
	void EditorLoadToWorld(EBlizzardState State);
	void EditorEnsureNormalBackup();
#endif

#if WITH_EDITORONLY_DATA
	/** 미리보기 전 자동 캡처한 평상시 백업 (세션 1회). 라이브 컴포넌트와 인덱스 대응.
	 *  Normal*Template 과 다른 것이다 — 이쪽은 "이번 세션 시작 시점으로 되돌리기"용이고 저장되지 않는다. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<USceneComponent>> EditorNormalBackups;
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<USceneComponent>> EditorBackupLiveComps;
#endif

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
	/** 전조 진입 시 문 달린 쉘터 중 OpenShelterCount 개를 랜덤으로 열고 나머지는 닫는다. */
	void OpenRandomShelterDoors();
	/** 모든 쉘터의 문을 닫는다 (눈보라 종료 / 기믹 비활성화). */
	void CloseAllShelterDoors();
	bool IsCharacterInShelter(const ACharacter* Character) const;

	/** BeginPlay 1회. 팅겨낼 목적지 후보를 모은다. */
	void GatherPlayerStarts();
	/** 눈보라 종료 시 쉘터 안에 있던 플레이어를 전부 밖(PlayerStart)으로 내보낸다. */
	void EjectCharactersFromShelters();
	void TeleportToRandomPlayerStart(ADefaultTromboneCharacter* Character);
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

	/** 서버 전용: 수집한 안전지대 목록 (StartWarning 마다 갱신) */
	TArray<TWeakObjectPtr<ABlizzardShelter>> Shelters;

	/** 서버 전용: 눈보라 종료 시 쉘터 점거자를 내보낼 목적지 (BeginPlay 1회 수집) */
	TArray<TWeakObjectPtr<APlayerStart>> CachedPlayerStarts;
};
