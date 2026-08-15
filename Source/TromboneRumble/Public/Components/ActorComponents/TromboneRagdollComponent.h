// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TromboneRagdollComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRagdollSignature);

USTRUCT(BlueprintType)
struct FRagdollNetState
{
	GENERATED_BODY()
	
	/** using FVector_NetQuantize for reduce bandwidth. */
	UPROPERTY()
	FVector_NetQuantize PelvisLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantize PelvisVelocity = FVector::ZeroVector;

	/** Server world time when this state was captured. */
	UPROPERTY()
	float Timestamp = 0.0f;
};

/** UTromboneRagdollComponent
 * 래그돌(물리 시뮬레이션) 중 위치 동기화를 수행하는 컴포넌트.
 * 서버의 골반 위치와 속도만 동기화하고, 회전은 동기화하지 않는다. 
 * 속도 보간을 적용함으로써 나머지 물리 바디가 자연스럽게 따라오도록 한다.
 */
UCLASS()
class TROMBONERUMBLE_API UTromboneRagdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:    
	
	/** Default Constructor */
	UTromboneRagdollComponent();

public:

	/** Start ragdoll. Server only.
	 *  @param InitialVelocity Initial speed.
	 *						   If non-zero, the existing speed is ignored and overwritten with this value.
	 *                         If set to "Zero" (default), maintains existing speed.
	 *  @param InitialAngularVelocity Initial spin (rad/s).
	 *                         If non-zero, the existing spin is ignored and overwritten with this value.
	 *                         If set to "Zero" (default), maintains existing spin. */
	void StartRagdoll(const FVector& InitialVelocity = FVector::ZeroVector, const FVector& InitialAngularVelocity = FVector::ZeroVector);
	
	/** Stop ragdoll and start get-up animation. Server only. */
	void StopRagdoll();
	
	bool IsRagdoll() const { return bIsRagdoll; }

	/** Server only. */
	void SetAutoGetUpEnabled(const bool bEnabled) { bAutoGetUpEnabled = bEnabled; }

	/** @return true if the ragdoll is at rest (pelvis speed below RestSpeedThreshold), otherwise false. */
	bool IsRagdollResting() const;

public:

	/** Event when ragdoll is started. */
	FRagdollSignature OnRagdollStarted;
	
	/** Event when ragdoll is ended. At this point, get-up animation is started and still simulating physics */
	FRagdollSignature OnRagdollEnded;
	
	/** Event when physics are enabled. */
	FRagdollSignature OnRagdollPhysicsEnabled;
	
protected:
	
	/** Ragdoll Duration (seconds) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "래그돌 지속 시간"))
	float RagdollDuration = 2.5f;
	
	/** The interpolation speed while ragdolling, to synchronize with the server's pelvis position */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "래그돌 중 메쉬의 속도 보간 속도"))
	float VelocityInterpSpeed = 15.0f;
	
	/** Network update rate per second */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "네트워크 업데이트 주기"))
	float PacketsPerSecond = 30.0f;

	/** 외삽 시 상한 시간으로, 패킷 손실 시 목표 위치가 너무 멀리 예측되는 것을 방지 */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "최대 외삽 시간"))
	float MaxExtrapolationTime = 0.25f;
	
	/** Squared distance threshold for forcing a hard location snap (cm^2) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "골반 위치 강제 동기화 거리"))
	float ForceLocationUpdateDistance = 40000.0f;
	
	/** Tracking intensity factor used to pull pelvis toward target position (P-Control) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "추적 강도"))
	float TrackingIntensity = 10.0f;

	/** Ragdoll rest speed threshold (cm/s) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "정지 판정 속도 임계값", ClampMin = "0.0"))
	float RestSpeedThreshold = 20.0f;

	/** Duration of the physics-to-animation blend-out after the get-up montage starts playing. */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "기상 애니메이션 블렌드 시간"))
	float RagdollBlendOutDuration = 0.2f;
	
	/** if true, enables visual debug and screen error logging
	 * When the ragdoll state begins or ends, print maximum difference in pelvis between the server and the client during the ragdoll state. */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "디버그 모드"))
	bool bEnableDebug = false;
	
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "래그돌 시 하늘로 날리기", EditCondition = "bEnableDebug"))
	bool bEnableImpulseOnRagdollStart = false;

	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "하늘로 날리는 힘", EditCondition = "bEnableImpulseOnRagdollStart"))
	float UpForce = 5000.f;
	
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "랜덤 XY 방향 범위", EditCondition = "bEnableImpulseOnRagdollStart"))
	float RandomRangeXY = 1500.f;
	
private:

	UFUNCTION()
	void OnRep_IsRagdoll();

	void UnapplyRagdoll();

	/** Starts the physics-to-animation blend-out (ragdoll bodies are still simulating at this point). */
	void BeginRagdollBlendOut();

	/** Ramps SetAllBodiesPhysicsBlendWeight from 1 (physics) to 0 (animation) over RagdollBlendOutDuration. */
	void TickRagdollBlendOut(float DeltaTime);

	/** Called once the blend-out reaches 0; hands off to UnapplyRagdoll for the final discrete cleanup. */
	void FinishRagdollBlendOut();

	/** @return true if the front of the pelvis is facing toward the sky, otherwise false. */
	bool IsFacingUp() const;

	void Server_ComputeGetUpTransform();

	void Server_UpdateRagdollTransform();
	
	void Client_InterpolateRagdollVelocity(float DeltaTime);

private:
	
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> OwnerMesh;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsRagdoll)
	bool bIsRagdoll = false;

	UPROPERTY(Replicated)
	FRagdollNetState ServerRagdollState;

	/** Authoritative get-up capsule location, computed once by the server */
	UPROPERTY(Replicated)
	FVector_NetQuantize GetUpLocation = FVector::ZeroVector;

	float TimeSinceLastNetUpdate = 0.0f;

	/** Time spent at rest during ragdoll */
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "RagdollComponent")
	float RagdollRestingTime = 0.0f;

	/** If false, does not automatically get up after resting. Server Only. */
	bool bAutoGetUpEnabled = true;

	/** Maximum difference for pelvis location synchronization (DebugMode) */
	float PelvisLocationMaxError = 0.0f;

	/** True while ramping SetAllBodiesPhysicsBlendWeight from 1 to 0 after the get-up montage has started. */
	bool bIsBlendingOut = false;

	/** Normalized [0,1] progress through the blend-out window. */
	float BlendOutAlpha = 0.0f;

public:
	
	// ~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// ~ End UActorComponent Interface
	
};
