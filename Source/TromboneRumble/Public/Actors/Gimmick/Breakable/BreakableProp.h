// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Breakable.h"
#include "BreakableProp.generated.h"

class UStaticMeshComponent;
class UGeometryCollectionComponent;
class UAkComponent;
class UAkAudioEvent;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPropBroken, ABreakableProp*, Prop, const FBreakHitInfo&, HitInfo);

/** ABreakableProp
 *
 * Chaos 파편으로 부서지는 배치 소품의 베이스. AGimmickBase가 아니다(GimmickManager는 타입당 1개만 등록한다).
 * 술잔·접시 같은 일반 소품은 C++ 파생 없이 이 클래스를 BP로 직접 상속해서 만든다.
 *
 * 네트워크: 서버가 Break()로 판정하고 bBroken + LastHit 만 복제한다.
 * 파편(GeometryCollection)은 각 머신이 로컬로 시뮬레이션하므로 파편 위치는 머신마다 다르다.
 *
 * 기본값은 일반 소품 기준(BreakableProp 프로파일 = 뚫림 + 닿거나 때리면 부서짐).
 * 문처럼 벽으로 막아야 하는 소품만 파생에서 덮어쓴다.
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API ABreakableProp : public AActor, public IBreakable
{
	GENERATED_BODY()

public:
	ABreakableProp();

	/** 부서진 직후 서버에서 브로드캐스트 */
	UPROPERTY(BlueprintAssignable, Category = "Breakable")
	FOnPropBroken OnBroken;

	//~ Begin IBreakable Interface
	virtual bool Break_Implementation(const FBreakHitInfo& HitInfo) override;
	virtual bool IsBroken_Implementation() const override;
	//~ End IBreakable Interface

protected:
	/** 부서지기 전의 온전한 메시. 루트이며 쿼리 콜리전과 오버랩의 출처 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> IntactMesh;

	/** 파편. 부서지기 전에는 숨김 + 시뮬레이션 꺼짐(물리 프록시조차 없음) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGeometryCollectionComponent> Debris;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAkComponent> AkComponent;

	/** 폰이 닿기만 해도 부서지는가 (술잔 true / 문 false) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable", meta = (DisplayName = "닿으면 부서짐"))
	bool bBreakOnPawnTouch = true;

	/** 무기 공격에 부서지는가 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable", meta = (DisplayName = "공격에 부서짐"))
	bool bBreakOnAttack = true;

	/** 파편을 정리하기까지의 시간 (초). GC 에셋의 Remove on Sleep 과 이중 안전장치 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable", meta = (DisplayName = "파편 수명", ClampMin = "1.0"))
	float DebrisLifetimeSeconds = 12.f;

	/** 파편 초기 속도 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable", meta = (DisplayName = "파편 속도 배율", ClampMin = "0.0"))
	float BreakSpeedScale = 1.f;

	/** 파편 초기 회전 속도 (rad/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable", meta = (DisplayName = "파편 회전 속도", ClampMin = "0.0"))
	float BreakSpinRadPerSec = 4.f;

	/** 충격점에서 파편을 밀어내는 반경 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable", meta = (DisplayName = "충격 반경", ClampMin = "0.0"))
	float DebrisImpulseRadius = 150.f;

	/** 닿아서 부서질 때 보장되는 최소 파편 속도 (cm/s). 가만히 서서 닿아도 파편이 튀게 한다 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable", meta = (DisplayName = "접촉 최소 속도", ClampMin = "0.0"))
	float MinTouchSpeed = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable|FX", meta = (DisplayName = "파괴 VFX"))
	TObjectPtr<UNiagaraSystem> BreakVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable|FX", meta = (DisplayName = "파괴 SFX"))
	TObjectPtr<UAkAudioEvent> BreakSoundEvent;

	UPROPERTY(ReplicatedUsing = OnRep_Broken)
	bool bBroken = false;

	UPROPERTY(Replicated)
	FBreakHitInfo LastHit;

	UFUNCTION()
	void OnRep_Broken();

	/** 각 머신의 로컬 연출. 서버/클라 모두 여기로 모인다 */
	void PlayBreakLocal();

	void CleanupDebris();

	UFUNCTION()
	void OnIntactMeshBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	/** 연출 1회 보장 (리슨 서버는 OnRep을 직접 부르므로 복제로 또 들어올 수 있다) */
	bool bBreakPlayed = false;

	FTimerHandle DebrisLifetimeHandle;

public:
	//~ Begin AActor Interface
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface
};
