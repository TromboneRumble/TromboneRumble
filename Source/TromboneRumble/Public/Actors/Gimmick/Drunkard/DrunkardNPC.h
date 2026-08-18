// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/TromboneCharacterBase.h"
#include "DrunkardNPC.generated.h"

class ADrunkardSpawner;
class UDrunkardDataAsset;
class UDrunkardStateComponent;
class UXRaySilhouetteComponent;

/** 취객 기믹 공용 로그 카테고리. 정의는 DrunkardNPC.cpp
 *  (파일별 DEFINE_LOG_CATEGORY_STATIC은 유니티 빌드에서 같은 청크에 묶이면 중복 정의로 충돌한다) */
DECLARE_LOG_CATEGORY_EXTERN(LogDrunkard, Log, All);

/** ADrunkardNPC
 *
 * 재즈바 취객 NPC 폰 본체. 피격(넉백/스턴/래그돌)은 베이스가 처리한다.
 * - 게임 규칙(상태/타겟/타이머)은 UDrunkardStateComponent 소유
 * - 이동은 AIController + BT (위빙은 BT 서비스에서 — Step 3)
 * - 상체 물리/전용 표정은 연출 레이어 (Step 4)
 */
UCLASS()
class TROMBONERUMBLE_API ADrunkardNPC : public ATromboneCharacterBase
{
	GENERATED_BODY()

public:
	ADrunkardNPC();

	//~ Begin ICombatReceiver Interface
	virtual bool OnHitReceived_Implementation(const FHitData& HitData) override;
	//~ End ICombatReceiver Interface

	void SetOwningSpawner(ADrunkardSpawner* InSpawner);
	ADrunkardSpawner* GetOwningSpawner() const;

	const UDrunkardDataAsset* GetDrunkardData() const { return DrunkardData; }
	UDrunkardStateComponent* GetStateComponent() const { return StateComponent; }

	//~ Begin ATromboneCharacterBase Interface
	virtual bool CanReceiveHit() const override;
	//~ End ATromboneCharacterBase Interface

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Data", meta = (DisplayName = "취객 데이터"))
	TObjectPtr<UDrunkardDataAsset> DrunkardData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UDrunkardStateComponent> StateComponent;

	/** 벽 뒤 실루엣. DrunkardData의 토글이 꺼져 있으면 BeginPlay에서 제거된다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UXRaySilhouetteComponent> XRaySilhouetteComponent;

private:
	/** 피격 경직(스턴) 시작/종료 시 이동을 정지/복원한다. AI는 입력 잠금의 영향을 받지 않으므로 속도로 제어 */
	UFUNCTION()
	void HandleStunStateChanged(bool bIsStunned);

	/** 상체(지정 본 이하) 한정 Physical Animation 적용 — "취함" 연출 레이어.
	 *  로컬 전용 연출(복제 없음, 캡슐/판정 무관). 래그돌 종료 시 OnRagdollPhysicsEnabled로 재적용된다.
	 *  본 트랜스폼 버퍼가 준비되지 않았으면(첫 포즈 평가 전) 다음 틱으로 연기한다 */
	UFUNCTION()
	void ApplyUpperBodyPhysics();

	int32 UpperBodyPhysicsRetryCount = 0;

	/** Trombone.Drunkard.Debug 1 활성 시 기믹 전 상태(상태/타겟/타이머/이동 목표)를 화면과 월드에 그린다 */
	void DebugDrawGimmickState() const;

	TWeakObjectPtr<ADrunkardSpawner> OwningSpawner;

	//~ Begin AActor Interface
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
		FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	//~ End AActor Interface
};
