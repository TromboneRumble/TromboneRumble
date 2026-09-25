// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/Breakable/BreakableProp.h"
#include "BreakablePhysicsProp.generated.h"

/** ABreakablePhysicsProp
 *
 * 닿으면 차여서 굴러가고, 세게 부딪히면 부서지는 소품 (접시·와인잔).
 * 무기에 맞으면 바로 부서진다. 차기·충돌 판정은 서버에서만 하고 위치는 복제 이동으로 맞춘다.
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API ABreakablePhysicsProp : public ABreakableProp
{
	GENERATED_BODY()

public:
	ABreakablePhysicsProp();

	//~ Begin IBreakable Interface
	virtual bool Break_Implementation(const FBreakHitInfo& HitInfo) override;
	//~ End IBreakable Interface

protected:
	/** 충돌 한 번의 속도 변화가 이 값 이상이면 부서진다 (cm/s). 테이블 높이 낙하 ≈ 420 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable|Physics", meta = (DisplayName = "파괴 충격 속도", ClampMin = "0.0"))
	float BreakImpactSpeed = 350.f;

	/** 찰 때 폰 속도에 곱하는 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable|Physics", meta = (DisplayName = "차기 배율", ClampMin = "0.0"))
	float KickSpeedScale = 0.8f;

	/** 가만히 서서 닿아도 보장되는 최소 차기 속도 (cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable|Physics", meta = (DisplayName = "최소 차기 속도", ClampMin = "0.0"))
	float MinKickSpeed = 200.f;

	/** 찰 때 위로 더하는 속도 (cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable|Physics", meta = (DisplayName = "차올림 속도", ClampMin = "0.0"))
	float KickUpSpeed = 150.f;

	/** 한 번 찬 뒤 다시 찰 수 있기까지의 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Breakable|Physics", meta = (DisplayName = "차기 쿨다운", ClampMin = "0.0"))
	float KickCooldown = 0.2f;

	UFUNCTION()
	void OnPhysicsBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPhysicsHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnPhysicsWake(UPrimitiveComponent* WakingComponent, FName BoneName);

	UFUNCTION()
	void OnPhysicsSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);

private:
	double LastKickTime = -1.0;

public:
	//~ Begin AActor Interface
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	//~ End AActor Interface
};
