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

	UPROPERTY()
	FQuat PelvisRotation = FQuat::Identity;

	UPROPERTY()
	FVector_NetQuantize PelvisAngularVelocity = FVector::ZeroVector;

	/** Server world time when this state was captured. */
	UPROPERTY()
	float Timestamp = 0.0f;
};

/** UTromboneRagdollComponent
 * Keeps a ragdoll (physics simulation) in the same place on every machine.
 * Only the server's pelvis location/rotation and their speeds are sent over.
 * The rest of the bodies are left to each machine's own simulation to follow along.
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

	/** Floats the ragdoll at the given world Z instead of letting it sink. Runs on every machine. */
	void SetFloatingEnabled(bool bEnabled, float InWaterLevelZ = 0.f);

	bool IsFloating() const { return bIsFloating; }

	/** Moves the surface the ragdoll floats at. Cheap - call it while the water rises or drains. */
	void SetWaterLevelZ(const float InWaterLevelZ) { WaterLevelZ = InWaterLevelZ; }

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
	
	/** The interpolation speed during ragdoll, to synchronize with the server's pelvis position */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "래그돌 중 메쉬의 속도 보간 속도"))
	float VelocityInterpSpeed = 15.0f;

	/** The interpolation speed during ragdoll, to synchronize with the server's pelvis rotation */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "래그돌 중 메쉬의 각속도 보간 속도"))
	float AngularVelocityInterpSpeed = 15.0f;
	
	/** Network update rate per second */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "네트워크 업데이트 주기"))
	float PacketsPerSecond = 30.0f;

	/** Maximum extrapolation time */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "최대 외삽 시간"))
	float MaxExtrapolationTime = 0.25f;
	
	/** Squared distance threshold for forcing a hard location snap (cm^2) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "골반 위치 강제 동기화 거리"))
	float ForceLocationUpdateDistance = 40000.0f;
	
	/** Tracking intensity factor used to pull pelvis toward target position (P-Control) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "추적 강도"))
	float TrackingIntensity = 10.0f;

	/** Tracking intensity factor used to pull pelvis toward target rotation (P-Control) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "회전 추적 강도"))
	float AngularTrackingIntensity = 10.0f;

	/** Limit for velocity correction (cm/s). */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "최대 보정 속도", ClampMin = "0.0"))
	float MaxCorrectionSpeed = 600.0f;

	/** Limit for angular velocity correction (rad/s). */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "최대 보정 각속도", ClampMin = "0.0"))
	float MaxCorrectionAngularSpeed = 12.0f;

	/** How far below the target the pelvis has to sink to count as stuck under the floor (cm). */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "관통 판정 깊이", ClampMin = "0.0"))
	float PenetrationDepthThreshold = 15.0f;

	/** How long it has to stay stuck before the body is lifted back up (seconds). */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "관통 복구 대기 시간", ClampMin = "0.0"))
	float PenetrationRecoverySeconds = 0.5f;

	/** Ragdoll rest speed threshold (cm/s) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "정지 판정 속도 임계값", ClampMin = "0.0"))
	float RestSpeedThreshold = 20.0f;

	/** Radius of the sphere the server sweeps below the pelvis to decide whether the body is airborne (cm). */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "지면 감지 반경", ClampMin = "1.0"))
	float GroundProbeRadius = 20.0f;

	/** How far below the pelvis that sphere is swept (cm). */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "지면 감지 거리", ClampMin = "1.0"))
	float GroundProbeDistance = 30.0f;

	/** Duration of the physics-to-animation blend-out after the get-up montage starts playing. */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "기상 애니메이션 블렌드 시간"))
	float RagdollBlendOutDuration = 0.2f;

	/** Upward pull on a fully submerged body. Must be larger than gravity (980) to lift it. */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent|Float", meta = (DisplayName = "부력 가속도", ClampMin = "0.0"))
	float BuoyancyAccel = 2200.f;

	/** Depth in cm where the pull reaches its full strength. Larger values let the body sink deeper before it comes back up. */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent|Float", meta = (DisplayName = "완전 침수 깊이", ClampMin = "1.0"))
	float FullSubmersionDepth = 30.f;

	UPROPERTY(EditAnywhere, Category = "RagdollComponent|Float", meta = (DisplayName = "물속 선형 저항", ClampMin = "0.0"))
	float WaterLinearDamping = 2.f;

	UPROPERTY(EditAnywhere, Category = "RagdollComponent|Float", meta = (DisplayName = "물속 회전 저항", ClampMin = "0.0"))
	float WaterAngularDamping = 2.f;
	
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "DEBUG: 하늘로 날리는 힘 (EnableImpulseOnStart)"))
	float UpForce = 4000.f;

	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "DEBUG: 랜덤 XY 방향 범위 (EnableImpulseOnStart)"))
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

	/** @return Whether the pelvis is up in the air. */
	bool IsPelvisAirborne() const;

	/** Snaps the body back to the target once the pelvis has stayed too far below it for too long. */
	void Client_RecoverFromGroundPenetration(float DeltaTime, const FVector& CurrentPelvisLoc, const FVector& TargetPelvisLoc, const FQuat& CurrentPelvisRot, const FQuat& TargetPelvisRot);

	/** Moves the whole body at once so that the pelvis lands on the target location and rotation.
	 *  How the arms and legs sit against each other is left as it was. */
	void SnapRagdollToTarget(const FVector& TargetPelvisLoc, const FQuat& TargetPelvisRot, const FVector& CurrentPelvisLoc, const FQuat& CurrentPelvisRot);
	
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

	/** Pushes every submerged body up. Called each tick while floating. */
	void ApplyBuoyancy() const;

	/** True while buoyancy is applied each tick. */
	bool bIsFloating = false;

	/** World Z the body floats toward. Only read while floating. */
	float WaterLevelZ = 0.f;

	/** Maximum difference for pelvis location synchronization (DebugMode) */
	float PelvisLocationMaxError = 0.0f;

	/** How long the body has been stuck under the floor (seconds). */
	float GroundPenetrationTime = 0.0f;

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
