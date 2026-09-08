// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/TromboneCharacterBase.h"
#include "DrunkardNPC.generated.h"

class ADefaultTromboneCharacter;
class ADrunkardSpawner;
class UDrunkardDataAsset;
class UDrunkardStateComponent;
class UWidgetComponent;
class UXRaySilhouetteComponent;

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

	/** 포획 성공 연출 시작 — 이동을 멈추고 다이브 몽타주를 재생한다. StateComponent가 호출. Server Only. */
	void BeginDive();

	/** 등장 연출 시작 — 문 뒤에서 실내까지 전방으로 통과 이동한다. StateComponent가 호출. Server Only. */
	void BeginDoorEntrance();

	/** 다이브 몽타주의 래그돌 시작 노티파이가 호출. 서버에서만 래그돌로 전환하고 복제로 퍼진다 */
	void HandleDiveRagdollStart();

	const UDrunkardDataAsset* GetDrunkardData() const { return DrunkardData; }
	UDrunkardStateComponent* GetStateComponent() const { return StateComponent; }

	//~ Begin ATromboneCharacterBase Interface
	virtual bool CanReceiveHit() const override;
	/** 다이브 래그돌에서 완전히 일어난 뒤 퇴장으로 잇는다 */
	virtual void HandleGetUpFinished() override;
	/** Stops moving while afloat. */
	virtual void HandleDrowningStarted() override;
	/** Does nothing - HandleGetUpFinished restores the speed once the get-up montage ends. */
	virtual void HandleDrowningEnded() override {}
	//~ End ATromboneCharacterBase Interface

protected:

	//~ Begin ATromboneCharacterBase Interface
	/** AI walks by speed, not by input, so the walk speed is what gets locked. */
	virtual void OnBlockedStateChanged(bool bBlocked) override;
	//~ End ATromboneCharacterBase Interface

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Data", meta = (DisplayName = "취객 데이터"))
	TObjectPtr<UDrunkardDataAsset> DrunkardData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UDrunkardStateComponent> StateComponent;

	/** 벽 뒤 실루엣. DrunkardData의 토글이 꺼져 있으면 BeginPlay에서 제거된다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UXRaySilhouetteComponent> XRaySilhouetteComponent;

	/** Portrait of the current target above the head. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UWidgetComponent> TargetIndicatorComponent;

	/** Attackable mark, head placement. Only the target sees it, and only within range. One of the two placements will be removed once the team picks. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UWidgetComponent> AttackableIndicatorHeadComponent;

	/** Attackable mark, chest placement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UWidgetComponent> AttackableIndicatorChestComponent;

private:

	/** Skin color of the current target. */
	UPROPERTY(ReplicatedUsing = OnRep_TargetSkinColor)
	FLinearColor TargetSkinColor = FLinearColor::Transparent;

	/** Current target. Clients use it only to know whether the local player is the one being chased. */
	UPROPERTY(ReplicatedUsing = OnRep_Target)
	TObjectPtr<ADefaultTromboneCharacter> ReplicatedTarget;

	UFUNCTION()
	void OnRep_TargetSkinColor();

	UFUNCTION()
	void OnRep_Target();

	/** Server. Copies the new target and its skin color into the replicated fields. */
	UFUNCTION()
	void HandleTargetChanged(ADefaultTromboneCharacter* NewTarget);

	/** Local. Shows the attackable mark while the local player is the target, close enough, and the drunkard can take a hit. */
	void UpdateAttackableIndicator();

	bool bAttackableShown = false;

	/** 상체(지정 본 이하) 한정 Physical Animation 적용 — "취함" 연출 레이어.
	 *  로컬 전용 연출(복제 없음, 캡슐/판정 무관). 래그돌 종료 시 OnRagdollPhysicsEnabled로 재적용된다.
	 *  본 트랜스폼 버퍼가 준비되지 않았으면(첫 포즈 평가 전) 다음 틱으로 연기한다 */
	UFUNCTION()
	void ApplyUpperBodyPhysics();

	int32 UpperBodyPhysicsRetryCount = 0;

	/** 문 통과 이동 상태. Tick에서 보간하고 끝나면 StateComponent에 알린다 */
	bool bDoorEntranceActive = false;
	FVector DoorEntranceStart = FVector::ZeroVector;
	FVector DoorEntranceEnd = FVector::ZeroVector;
	float DoorEntranceElapsed = 0.f;

	/** 다이브 몽타주는 NPC라 자동 복제가 안 되므로 모든 머신에서 직접 재생한다 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDiveMontage();

	/** 래그돌 시작 시 상체 물리 모터를 끈다. 켜둔 채 두면 월드 공간 모터가
	 *  얼어붙은 캡슐 위치의 애니메이션 포즈로 몸을 끌어당겨 래그돌이 공중에 매달린다 */
	UFUNCTION()
	void ClearUpperBodyPhysics();

	/** Trombone.Drunkard.Debug 1 활성 시 기믹 전 상태(상태/타겟/타이머/이동 목표)를 화면과 월드에 그린다 */
	void DebugDrawGimmickState() const;

	TWeakObjectPtr<ADrunkardSpawner> OwningSpawner;

	//~ Begin AActor Interface
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
		FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface
};
