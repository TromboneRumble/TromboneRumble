// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/Bonus/BonusDrop.h"
#include "Present.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class URotatingMovementComponent;

/**
 * Present is the gift of the snow field that hangs from a balloon.
 * It swings down like a pendulum, and once it lands it pops in, floats on a wave and spins until a player picks it up.
 * ABonusSpawner drops it on a timer, and APressurePlate_SpawnPresent drops one when a player steps on the plate.
 *
 * @see ABonusSpawner
 * @see APressurePlate_SpawnPresent
 */
UCLASS()
class TROMBONERUMBLE_API APresent : public ABonusDrop
{
	GENERATED_BODY()

public:

	APresent();

	//~ Begin AActor Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

protected:

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface

	//~ Begin ABonusDrop Interface
	virtual void TickFalling(float DeltaTime) override;
	virtual void OnLanded() override;
	virtual void TickLanded(float DeltaTime) override;
	//~ End ABonusDrop Interface

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GiftMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BalloonMesh;

	/** 진자 회전 중심 (풍선과 선물을 연결하는 축) */
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> PendulumPivot;

	/** 착지 후 선물 주변에서 계속 재생되는 반짝임 (에셋 미지정이면 아무 일도 하지 않음) */
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UNiagaraComponent> SparkleVFX;

	/** 착지 후 월드 Z축 스핀. 낙하 중에는 비활성 (bAutoActivate = false) */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<URotatingMovementComponent> RotatingMovement;

	/** 진자 최대 흔들림 각도 (도) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "진자 최대 각도 (도)"))
	float MaxPendulumAngleDeg = 30.f;

	/** 진자 길이 - 풍선과 선물 사이 거리 (cm) */
	UPROPERTY(VisibleInstanceOnly, Category = "Present|Config", meta = (DisplayName = "계산된 진자 길이 (cm)"))
	float PendulumLength;

	/** 흔들림 주기 (Hz) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "흔들림 주기 (Hz)"))
	float SwayFrequency = 1.2f;

	/** 낙하 가속도 (cm/s²) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "낙하 가속도"))
	float GravityAccel = 300.f;

	/** 초기 낙하 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "초기 낙하 속도"))
	float InitialFallSpeed = 50.f;

	/** 접지 지점에서 띄울 높이 (cm, 월드 Z) */
	UPROPERTY(EditAnywhere, Category = "Present|Landing", meta = (DisplayName = "공중 부양 높이 (cm)"))
	float HoverHeight = 40.f;

	/** 팝인 시 순간적으로 부풀어 오르는 최대 배율 */
	UPROPERTY(EditAnywhere, Category = "Present|Landing", meta = (DisplayName = "팝인 최대 배율"))
	float PopOvershootScale = 1.2f;

	/** 스케일 0 → 최대 배율까지 걸리는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Present|Landing", meta = (DisplayName = "팝인 확대 시간 (초)"))
	float PopGrowDuration = 0.35f;

	/** 최대 배율 → 원래 크기로 돌아오는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Present|Landing", meta = (DisplayName = "팝인 복원 시간 (초)"))
	float PopSettleDuration = 0.25f;

	/** 착지 후 기울기 Pitch (도) */
	UPROPERTY(EditAnywhere, Category = "Present|Landing", meta = (DisplayName = "기울기 Pitch (도)"))
	float TiltPitchDeg = 20.f;

	/**
	 * 착지 후 기울기 Roll (도).
	 * 회전 속도는 RotatingMovement 컴포넌트의 Rotation Rate에서 조정한다.
	 */
	UPROPERTY(EditAnywhere, Category = "Present|Landing", meta = (DisplayName = "기울기 Roll (도)"))
	float TiltRollDeg = 15.f;

	/** 파형 진행 속도 (rad/s) */
	UPROPERTY(EditAnywhere, Category = "Present|Wave", meta = (DisplayName = "출렁임 속도"))
	float WaveSpeed = 1.6f;

	/** 위아래 출렁임 진폭 (cm). 바닥을 뚫지 않도록 공중 부양 높이보다 작게 둘 것 */
	UPROPERTY(EditAnywhere, Category = "Present|Wave", meta = (DisplayName = "출렁임 높이 (cm)"))
	float WaveHeight = 10.f;

	/** 1이면 부드러운 파형, 클수록 마루가 뾰족하고 골이 넓고 평평해진다 (파도 느낌) */
	UPROPERTY(EditAnywhere, Category = "Present|Wave", meta = (DisplayName = "마루 날카로움", ClampMin = "1.0"))
	float WaveCrestSharpness = 1.8f;

	/** 출렁임에 따른 Pitch 흔들림 진폭 (도) */
	UPROPERTY(EditAnywhere, Category = "Present|Wave", meta = (DisplayName = "Pitch 흔들림 (도)"))
	float WavePitchWobbleDeg = 8.f;

	/** 출렁임에 따른 Roll 흔들림 진폭 (도) */
	UPROPERTY(EditAnywhere, Category = "Present|Wave", meta = (DisplayName = "Roll 흔들림 (도)"))
	float WaveRollWobbleDeg = 6.f;

	/** 팝인 순간 1회 재생되는 버스트 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Present|VFX", meta = (DisplayName = "팝인 버스트 VFX"))
	TObjectPtr<UNiagaraSystem> PopBurstVFX;

	/**
	 * SparkleVFX를 주기적으로 다시 재생시키는 간격 (초). 기본값 0 = 반복하지 않음.
	 * 루프형(Loop Behavior = Infinite) 시스템에는 쓰지 말 것 — 매번 리셋되어 연출이 끊긴다.
	 * 버스트형 시스템밖에 없을 때만 값을 올려 반복 재생을 흉내낸다.
	 */
	UPROPERTY(EditAnywhere, Category = "Present|VFX", meta = (DisplayName = "반짝임 반복 간격 (초)", ClampMin = "0.0"))
	float SparkleRepeatInterval = 0.f;

private:
	void PlayLandingVFX();

	UFUNCTION()
	void RestartSparkleVFX();

	UPROPERTY(Replicated)
	FVector AnchorSpawnLocation;

	UPROPERTY(Replicated)
	FVector ReplicatedSwayAxis;

	UPROPERTY(Replicated)
	float ReplicatedSwayPhaseOffset;

	FTimerHandle SparkleRepeatTimerHandle;

	float SwayTime = 0.f;

	/** 착지 후 경과 시간. 팝인 진행도와 출렁임 위상에 함께 쓰인다 (복제하지 않는 순수 로컬 연출값) */
	float LandedElapsed = 0.f;
};
