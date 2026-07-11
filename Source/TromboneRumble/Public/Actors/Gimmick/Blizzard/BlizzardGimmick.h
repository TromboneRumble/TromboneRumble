// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameplayEffectTypes.h"
#include "BlizzardGimmick.generated.h"

class UGameplayEffect;
class ACharacter;
class ADefaultTromboneCharacter;
class ATromboneCharacterBase;
class ABlizzardShelter;

UENUM(BlueprintType)
enum class EBlizzardState : uint8
{
	Idle,
	Warning,
	Active,
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
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** 상태 전이 시 클라/서버 양쪽에서 호출된다. VFX/사운드/방향 안내 연출을 BP에서 구현한다.
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
