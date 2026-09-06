// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Misc/App.h"
#include "Net/UnrealNetwork.h"

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CVarRagdollEnableImpulseOnStart(
	TEXT("Trombone.Ragdoll.EnableImpulseOnStart"),
	0,
	TEXT("If 1, character is launched when ragdoll start"));

static TAutoConsoleVariable<int32> CVarRagdollSyncFeatures(
	TEXT("Trombone.Ragdoll.SyncFeatures"),
	static_cast<int32>(ERagdollSyncFeature::All),
	TEXT("Bitmask of client sync parts. 1 rotation snap, 8 extrapolation, 16 rotation sync, 32 gravity term. 57 = all"));
#endif

namespace
{
	bool IsRagdollImpulseOnStartEnabled()
	{
#if !UE_BUILD_SHIPPING
		return CVarRagdollEnableImpulseOnStart.GetValueOnGameThread() != 0;
#else
		return false;
#endif
	}
}

int32 UTromboneRagdollComponent::GetSyncFeatures()
{
#if !UE_BUILD_SHIPPING
	return CVarRagdollSyncFeatures.GetValueOnGameThread();
#else
	return static_cast<int32>(ERagdollSyncFeature::All);
#endif
}

UTromboneRagdollComponent::UTromboneRagdollComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; 
	SetIsReplicatedByDefault(true);
}

void UTromboneRagdollComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	OwnerMesh = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;

	/** 서버에서 이미 래그돌 중인 폰이 리플리케이션으로 스폰되면(늦은 합류 등) OnRep이 BeginPlay보다 먼저 와서 무시되므로 
	 *	여기서 재적용. 캐릭터의 OnRagdollStarted 바인딩이 이 함수 이후에 이뤄지므로 반드시 다음 틱에 실행. */
	if (bIsRagdoll)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (bIsRagdoll)
			{
				OnRep_IsRagdoll();
			}
		}));
	}
}

void UTromboneRagdollComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	bIsBlendingOut = false;

	Super::EndPlay(EndPlayReason);
}

void UTromboneRagdollComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Runs on every machine, not just the server, so clients move in the water on their own
	if (bIsFloating && bIsRagdoll)
	{
		ApplyBuoyancy();
	}

	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		if (bIsRagdoll)
		{
			if (IsRagdollResting())
			{
				RagdollRestingTime += DeltaTime;
				if (bAutoGetUpEnabled && RagdollRestingTime >= RagdollDuration)
				{
					StopRagdoll();
				}
			}
			else
			{
				RagdollRestingTime = 0.0f;
			}
		}

		TimeSinceLastNetUpdate += DeltaTime;
		if (TimeSinceLastNetUpdate >= 1.0f / PacketsPerSecond)
		{
			TimeSinceLastNetUpdate = 0.0f;
			Server_UpdateRagdollTransform();
		}
	}
	else if (!bIsBlendingOut)
	{
		Client_InterpolateRagdollVelocity(DeltaTime);
	}

	if (bIsBlendingOut)
	{
		TickRagdollBlendOut(DeltaTime);
	}
}

void UTromboneRagdollComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsRagdoll);
	DOREPLIFETIME(ThisClass, ServerRagdollState);
	DOREPLIFETIME(ThisClass, RagdollStartServerTime);
	DOREPLIFETIME(ThisClass, GetUpLocation);
}

void UTromboneRagdollComponent::SetFloatingEnabled(const bool bEnabled, const float InWaterLevelZ)
{
	bIsFloating = bEnabled;
	WaterLevelZ = InWaterLevelZ;

	if (!OwnerMesh) return;

	const float NewLinear = bEnabled ? WaterLinearDamping : 0.f;
	const float NewAngular = bEnabled ? WaterAngularDamping : 0.f;
	OwnerMesh->ForEachBodyBelow(NAME_None, true, false,
		[NewLinear, NewAngular](FBodyInstance* Body)
		{
			Body->LinearDamping = NewLinear;
			Body->AngularDamping = NewAngular;
			Body->UpdateDampingProperties();
		});
}

void UTromboneRagdollComponent::ApplyBuoyancy() const
{
	if (!OwnerMesh) return;

	const float WaterZ = WaterLevelZ;
	const float Accel = BuoyancyAccel;
	const float FullDepth = FMath::Max(1.f, FullSubmersionDepth);

	OwnerMesh->ForEachBodyBelow(NAME_None, true, false,
		[WaterZ, Accel, FullDepth](FBodyInstance* Body)
		{
			const float Depth = WaterZ - Body->GetUnrealWorldTransform().GetLocation().Z;
			if (Depth <= 0.f) return;

			const float Submersion = FMath::Clamp(Depth / FullDepth, 0.f, 1.f);
			Body->AddForce(FVector::UpVector * Accel * Submersion, true, true);
		});
}

void UTromboneRagdollComponent::StartRagdoll(const FVector& InitialVelocity, const FVector& InitialAngularVelocity)
{
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(GetOwner());
		OwnerMesh = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	}

	if (!OwnerCharacter || !OwnerMesh || !OwnerCharacter->HasAuthority() || bIsRagdoll)
	{
		return;
	}

	RagdollRestingTime = 0.0f;

	// Clients ignore the state left from an earlier ragdoll until one newer than this arrives
	if (const AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		RagdollStartServerTime = GameState->GetServerWorldTimeSeconds();
	}

	bIsRagdoll = true;
	OnRep_IsRagdoll();

	if (!InitialVelocity.IsNearlyZero())
	{
		OwnerMesh->SetAllPhysicsLinearVelocity(InitialVelocity);
	}

	if (!InitialAngularVelocity.IsNearlyZero())
	{
		OwnerMesh->SetAllPhysicsAngularVelocityInRadians(InitialAngularVelocity);
	}

	if (IsRagdollImpulseOnStartEnabled())
	{
		const float RandomX = FMath::FRandRange(-RandomRangeXY, RandomRangeXY);
		const float RandomY = FMath::FRandRange(-RandomRangeXY, RandomRangeXY);

		const FVector LaunchVelocity = FVector(RandomX, RandomY, UpForce);
		OwnerMesh->AddImpulse(LaunchVelocity, TromboneBones::Pelvis, true);
	}
}

void UTromboneRagdollComponent::StopRagdoll()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !bIsRagdoll)
	{
		return;
	}

	Server_ComputeGetUpTransform();
	
	RagdollRestingTime = 0.0f;

	bIsRagdoll = false;
	OnRep_IsRagdoll();

	OwnerCharacter->ForceNetUpdate();
}

void UTromboneRagdollComponent::Server_ComputeGetUpTransform()
{
	const FVector PelvisLocation = OwnerMesh->GetSocketLocation(TromboneBones::Pelvis);

	FVector TargetCapsuleLocation = PelvisLocation;

	const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FHitResult HitResult;
	const FVector Start = PelvisLocation;
	const FVector End = PelvisLocation - FVector(0.0f, 0.0f, CapsuleHalfHeight * 2.0f);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		TargetCapsuleLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, CapsuleHalfHeight + 2.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[RagdollComponent::Server_ComputeGetUpTransform] Failed to find ground for capsule placement after ragdoll. Using pelvis location(%s) as fallback."), *PelvisLocation.ToString());
	}

	GetUpLocation = TargetCapsuleLocation;
}

void UTromboneRagdollComponent::OnRep_ServerRagdollState()
{
	// World time would still be last frame's here (LevelTick.cpp: TickDispatch runs before TimeSeconds advances)
	StateReceivedRealTime = FApp::GetCurrentTime();
}

void UTromboneRagdollComponent::OnRep_IsRagdoll()
{
	if (!OwnerCharacter || !OwnerMesh)
	{
		return;
	}
	
	if (bIsRagdoll)
	{
		SyncStats = FRagdollSyncStats();
		RotationSnapTime = 0.0f;
	}

	SetComponentTickEnabled(bIsRagdoll || bIsBlendingOut);
	
	if (bIsRagdoll)
	{
		bIsBlendingOut = false;
		BlendOutAlpha = 0.0f;

		OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		/** 래그돌 동안 이동 리플리케이션을 끈다.
		 *  켜 두면 Simulated Proxy에 이동 업데이트가 도착할 때마다 CMC가 캡슐을 서버 위치로 되돌려 로컬 캡슐 스냅을 무효화하고,
		 *  캡슐 위치 차이로 bJustTeleported = true가 되면 TeleportPhysics 이동이 된다.
		 *  메시는 캡슐에 부착된 자식이므로 부모가 움직이면 자식의 트랜스폼도 업데이트 되고 텔레포트 타입도 그대로 전달된다.
		 *  이때 UpdateKinematicBonesToAnim(PhysAnim.cpp)에서는 TeleportPhysics면 시뮬레이션 중인 래그돌 바디도 강제로 위치를 변경해버린다.
		 *  이 변경되는 위치는 지난 프레임의 애니메이션 포즈(래그돌 중엔 갱신이 안 된 Stale 데이터)인지라 메시가 튀는 현상이 발생한다.
		 *
		 *  래그돌 위치 동기화는 이 컴포넌트의 골반 동기화가, 기상 위치는 GetUpLocation이 담당하므로 이동 리플리케이션은 래그돌 동안 불필요하다.
		 */
		OwnerCharacter->SetReplicateMovement(false);
		
		/** 서버에서 원격 클라이언트가 조종하는 캐릭터는 서버 이동 패킷이 올 때만 애니메이션을 틱하는데,
		 * ([ACharacter::PossessedBy]에서 bOnlyAllowAutonomousTickPose가 true로 설정. 엔진 단에서의 서버 연산 최적화)
		 * 래그돌/기상 애니메이션 재생 중에는 클라이언트가 MOVE_None이라 이동 패킷을 안 보내기 때문에
		 * 서버에서 해당 캐릭터의 애니메이션 업데이트가 멈춰 기상 몽타주가 0초 시점에서 T포즈가 보여진다.
		 * 래그돌 중에는 일반 컴포넌트 틱으로 애니메이션이 연산되도록 한다.
		 */
		OwnerMesh->bOnlyAllowAutonomousTickPose = false;
		
		OwnerMesh->SetSimulatePhysics(true);
		OwnerMesh->SetEnableGravity(true);
		OwnerMesh->SetAllBodiesPhysicsBlendWeight(1.0f);
		OwnerMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		OwnerMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		OwnerMesh->SetAllUseCCD(true);
		GroundPenetrationTime = 0.0f;
		OnRagdollStarted.Broadcast();
	}
	else
	{
		const FRotator PelvisRotation = OwnerMesh->GetSocketRotation(TromboneBones::Pelvis);

		float TargetYaw = OwnerCharacter->GetActorRotation().Yaw;
		FVector PelvisForward = FRotationMatrix(PelvisRotation).GetScaledAxis(EAxis::X);
		PelvisForward.Z = 0.0f;
		if (!PelvisForward.IsNearlyZero(0.1f))
		{
			TargetYaw = PelvisForward.Rotation().Yaw + 90.0f;
		}
		
		const FRotator TargetCapsuleRotation(0.0f, TargetYaw, 0.0f);
		OwnerCharacter->SetActorLocationAndRotation(GetUpLocation, TargetCapsuleRotation, false, nullptr, ETeleportType::None);

		const FBodyInstance* PelvisBody = OwnerMesh->GetBodyInstance(TromboneBones::Pelvis);
		const FBodyInstance* RootBody = OwnerMesh->GetBodyInstance();
		if (PelvisBody && RootBody)
		{
			const FVector CurrentPelvisLoc = PelvisBody->GetUnrealWorldTransform().GetLocation();
			FVector DesiredPelvisLoc = CurrentPelvisLoc;
			DesiredPelvisLoc.X = GetUpLocation.X;
			DesiredPelvisLoc.Y = GetUpLocation.Y;

			const FVector CurrentRootBodyLoc = RootBody->GetUnrealWorldTransform().GetLocation();
			const FVector PelvisOffset = CurrentPelvisLoc - CurrentRootBodyLoc;
			OwnerMesh->SetAllPhysicsPosition(DesiredPelvisLoc - PelvisOffset);
		}
		
		if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(OwnerMesh->GetAnimInstance()))
		{
			AnimInst->PlayGetUpMontage(IsFacingUp());
		}

		BeginRagdollBlendOut();

		OnRagdollEnded.Broadcast();
	}
}

void UTromboneRagdollComponent::BeginRagdollBlendOut()
{
	bIsBlendingOut = true;
	BlendOutAlpha = 0.0f;
	SetComponentTickEnabled(true);

	OwnerMesh->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	OwnerMesh->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector);
	OwnerMesh->SetEnableGravity(false);
}

void UTromboneRagdollComponent::TickRagdollBlendOut(const float DeltaTime)
{
	if (RagdollBlendOutDuration <= KINDA_SMALL_NUMBER)
	{
		FinishRagdollBlendOut();
		return;
	}

	BlendOutAlpha = FMath::Clamp(BlendOutAlpha + DeltaTime / RagdollBlendOutDuration, 0.0f, 1.0f);
	
	const float PhysicsWeight = FMath::Lerp(1.0f, 0.0f, BlendOutAlpha);
	OwnerMesh->SetAllBodiesPhysicsBlendWeight(PhysicsWeight);

	if (BlendOutAlpha >= 1.0f)
	{
		FinishRagdollBlendOut();
	}
}

void UTromboneRagdollComponent::FinishRagdollBlendOut()
{
	bIsBlendingOut = false;
	OwnerMesh->SetAllBodiesPhysicsBlendWeight(0.0f);
	
	UnapplyRagdoll();
	SetComponentTickEnabled(bIsRagdoll || bIsBlendingOut);
}

void UTromboneRagdollComponent::UnapplyRagdoll()
{
	OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	OwnerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;

	OwnerMesh->SetSimulatePhysics(false);
	OwnerMesh->SetAllUseCCD(false);
	OwnerMesh->SetCollisionObjectType(ECC_Pawn);
	OwnerMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OwnerMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	OwnerMesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0.0f, -90.0f, 0.0f));

	if (OwnerCharacter->HasAuthority())
	{
		OwnerCharacter->SetReplicateMovement(true);

		if (OwnerCharacter->GetRemoteRole() == ROLE_AutonomousProxy && OwnerCharacter->GetNetConnection() != nullptr)
		{
			OwnerMesh->bOnlyAllowAutonomousTickPose = true;
		}
	}

	OnRagdollPhysicsEnabled.Broadcast();
}

bool UTromboneRagdollComponent::IsFacingUp() const
{
	if (!OwnerMesh) return true;

	const FRotator PelvisRotation = OwnerMesh->GetSocketRotation(TromboneBones::Pelvis);
	const FVector PelvisUp = FRotationMatrix(PelvisRotation).GetScaledAxis(EAxis::Z);

	return (FVector::DotProduct(PelvisUp, FVector::UpVector) > 0.0f);
}

bool UTromboneRagdollComponent::IsRagdollResting() const
{
	if (!OwnerMesh) return false;
	
	const FVector PelvisVelocity = OwnerMesh->GetPhysicsLinearVelocity(TromboneBones::Pelvis);
	return PelvisVelocity.SizeSquared() < FMath::Square(RestSpeedThreshold);
}

void UTromboneRagdollComponent::Server_UpdateRagdollTransform()
{
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}
	
	const FTransform PelvisTransform = OwnerMesh->GetBodyInstance(TromboneBones::Pelvis)->GetUnrealWorldTransform();

	FRagdollNetState NewState;
	NewState.PelvisLocation = PelvisTransform.GetLocation();
	NewState.PelvisVelocity = OwnerMesh->GetPhysicsLinearVelocity(TromboneBones::Pelvis);
	NewState.PelvisRotation = PelvisTransform.GetRotation();
	NewState.PelvisAngularVelocity = OwnerMesh->GetPhysicsAngularVelocityInRadians(TromboneBones::Pelvis);
	if (const AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		NewState.Timestamp = GameState->GetServerWorldTimeSeconds();
	}

	ServerRagdollState = NewState;
}

bool UTromboneRagdollComponent::IsPelvisAirborne() const
{
	const UWorld* World = GetWorld();
	const FBodyInstance* PelvisBody = OwnerMesh ? OwnerMesh->GetBodyInstance(TromboneBones::Pelvis) : nullptr;
	if (!World || !PelvisBody)
	{
		return false;
	}

	// Physics only puts a body to sleep once it has stopped moving, so a sleeping body is resting on something.
	if (!PelvisBody->IsInstanceAwake())
	{
		return false;
	}

	const FVector PelvisLocation = PelvisBody->GetUnrealWorldTransform().GetLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	return !World->SweepTestByChannel(
		PelvisLocation,
		PelvisLocation - FVector(0.0f, 0.0f, GroundProbeDistance),
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(GroundProbeRadius),
		QueryParams);
}

void UTromboneRagdollComponent::Client_InterpolateRagdollVelocity(const float DeltaTime)
{
    if (!OwnerMesh)
    {
	    UE_LOG(LogTemp, Warning, TEXT("[RagdollComponent::Client_InterpolatePelvisVelocity] SkeletalMeshComponent is NULL"));
    	return;
    }
	
    const FBodyInstance* PelvisBody = OwnerMesh->GetBodyInstance(TromboneBones::Pelvis);
    if (!PelvisBody)
    {
	    UE_LOG(LogTemp, Warning, TEXT("[RagdollComponent::Client_InterpolatePelvisVelocity] Pelvis BodyInstance is NULL"));
    	return;
    }

	// Nothing to follow yet. The state still belongs to an earlier ragdoll
	if (ServerRagdollState.Timestamp < RagdollStartServerTime)
	{
		return;
	}

    const FTransform CurrentPelvisTrans = PelvisBody->GetUnrealWorldTransform();
    const FVector CurrentPelvisLoc = CurrentPelvisTrans.GetLocation();
    const FQuat CurrentPelvisRot = CurrentPelvisTrans.GetRotation();

	const UWorld* World = GetWorld();
	float PacketAge = 0.0f;
	if (World && ServerRagdollState.Timestamp > 0.0f && IsSyncFeatureEnabled(ERagdollSyncFeature::Extrapolation)) // measurement switch
	{		
		/** 클라이언트 자기 시계만 쓴다. GetServerWorldTimeSeconds는 RTT/2가 아니라 게임스테이트 복제 주기 + 한 프레임만큼
		 *  뒤처져 있어서, "추정 서버시간 - 타임스탬프"가 음수가 되고 0으로 잘렸다.
		 *  상태를 받은 뒤 흐른 시간에는 그런 편향이 없다. 서버에서 오는 전송 시간은 아래에서 더한다. */
		PacketAge = static_cast<float>(FApp::GetCurrentTime() - StateReceivedRealTime);

		if (const APlayerController* LocalPC = World->GetFirstPlayerController())
		{
			if (const APlayerState* LocalPlayerState = LocalPC->PlayerState)
			{
				// 엔진에서 핑 == RTT. '서버->클라'와 '클라->서버' 편도 시간이 다를 수 있으니 이 값은 RTT/2는 '서버->클라' 편도 지연의 근사값
				PacketAge += LocalPlayerState->GetPingInMilliseconds() * 0.001f * 0.5f;
			}
		}

		PacketAge = FMath::Clamp(PacketAge, 0.0f, MaxExtrapolationTime);
	}

	const FVector ServerVelocity = ServerRagdollState.PelvisVelocity;
	FVector TargetPelvisLoc = FVector(ServerRagdollState.PelvisLocation) + ServerVelocity * PacketAge;

	const bool bAirborne = IsPelvisAirborne();

	// 중력가속도(등가속 운동) 반영 (measurement switch)
	if (World && bAirborne && IsSyncFeatureEnabled(ERagdollSyncFeature::GravityTerm))
	{
		TargetPelvisLoc.Z += 0.5f * World->GetGravityZ() * PacketAge * PacketAge;
	}

	// 각속도 * 시간 = 그동안 돌아간 양
	const FVector ServerAngularVelocity = ServerRagdollState.PelvisAngularVelocity;
	FQuat TargetPelvisRot = ServerRagdollState.PelvisRotation;
	if (const float SpinAngle = ServerAngularVelocity.Size() * PacketAge; SpinAngle > UE_KINDA_SMALL_NUMBER)
	{
		TargetPelvisRot = FQuat(ServerAngularVelocity.GetSafeNormal(), SpinAngle) * TargetPelvisRot;
	}
	TargetPelvisRot.Normalize();
	
	SyncStats.bHasState = true;
	SyncStats.StateTimestamp = ServerRagdollState.Timestamp;
	SyncStats.TargetPelvisLocation = TargetPelvisLoc;
	SyncStats.TargetPelvisRotation = TargetPelvisRot;
	SyncStats.PacketAge = PacketAge;
	SyncStats.bAirborne = bAirborne;

	/** 너무 오래, 너무 많이 틀어져 있으면 스냅한다. 바닥에서는 밀어서 되돌아오지 못하기 때문이다 (measurement switch).
	 *  외삽한 목표 회전이 아니라 서버 회전 그대로와 비교한다. 지연이 크면 각속도 외삽이 목표를
	 *  한두 프레임 흔드는데, 거기에 스냅하면 아무 이유 없이 몸이 튄다. */
	const float RawRotationErrorDeg = FMath::RadiansToDegrees(CurrentPelvisRot.AngularDistance(ServerRagdollState.PelvisRotation));
	RotationSnapTime = RawRotationErrorDeg >= ForceRotationUpdateAngle ? RotationSnapTime + DeltaTime : 0.0f;
	const bool bRotationSnap = IsSyncFeatureEnabled(ERagdollSyncFeature::RotationSnap) && RotationSnapTime >= RotationSnapHoldSeconds;

	// Too far away
    if (FVector::DistSquared(CurrentPelvisLoc, TargetPelvisLoc) > ForceLocationUpdateDistance || bRotationSnap)
    {
        SnapRagdollToTarget(TargetPelvisLoc, TargetPelvisRot, CurrentPelvisLoc, CurrentPelvisRot);
        GroundPenetrationTime = 0.0f;
        RotationSnapTime = 0.0f;
        ++SyncStats.SnapCount;
        return;
    }

    Client_RecoverFromGroundPenetration(DeltaTime, CurrentPelvisLoc, TargetPelvisLoc, CurrentPelvisRot, TargetPelvisRot);

    FVector PositionCorrection = (TargetPelvisLoc - CurrentPelvisLoc) * TrackingIntensity;
    PositionCorrection = PositionCorrection.GetClampedToMaxSize(MaxCorrectionSpeed);

	// Drop the downward part of the correction while the body is on the ground.
	if (!bAirborne && PositionCorrection.Z < 0.0f)
    {
        PositionCorrection.Z = 0.0f;
    }
	SyncStats.CorrectionSpeed = PositionCorrection.Size();

    const FVector TargetVelocity = ServerVelocity + PositionCorrection;
    const FVector CurrentVelocity = OwnerMesh->GetPhysicsLinearVelocity(TromboneBones::Pelvis);
    const FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, VelocityInterpSpeed);

    OwnerMesh->SetPhysicsLinearVelocity(NewVelocity, false, TromboneBones::Pelvis);

	// measurement switch
	if (!IsSyncFeatureEnabled(ERagdollSyncFeature::RotationSync))
	{
		return;
	}

    FQuat RotationError = TargetPelvisRot * CurrentPelvisRot.Inverse();
    RotationError.Normalize();
    if (RotationError.W < 0.0f)
    {
        // 같은 회전을 가리키는 두 부호 중 짧게 도는 쪽을 고른다
        RotationError = -RotationError;
    }

    FVector ErrorAxis;
    float ErrorAngle;
    RotationError.ToAxisAndAngle(ErrorAxis, ErrorAngle);

    FVector RotationCorrection = ErrorAxis * ErrorAngle * AngularTrackingIntensity;
    RotationCorrection = RotationCorrection.GetClampedToMaxSize(MaxCorrectionAngularSpeed);

    const FVector TargetAngularVelocity = ServerAngularVelocity + RotationCorrection;
    const FVector CurrentAngularVelocity = OwnerMesh->GetPhysicsAngularVelocityInRadians(TromboneBones::Pelvis);
    const FVector NewAngularVelocity = FMath::VInterpTo(CurrentAngularVelocity, TargetAngularVelocity, DeltaTime, AngularVelocityInterpSpeed);

    OwnerMesh->SetPhysicsAngularVelocityInRadians(NewAngularVelocity, false, TromboneBones::Pelvis);
}

void UTromboneRagdollComponent::Client_RecoverFromGroundPenetration(const float DeltaTime, const FVector& CurrentPelvisLoc, const FVector& TargetPelvisLoc,
	const FQuat& CurrentPelvisRot, const FQuat& TargetPelvisRot)
{
	const float PenetrationDepth = TargetPelvisLoc.Z - CurrentPelvisLoc.Z;
	if (PenetrationDepth < PenetrationDepthThreshold)
	{
		GroundPenetrationTime = 0.0f;
		return;
	}

	GroundPenetrationTime += DeltaTime;
	if (GroundPenetrationTime < PenetrationRecoverySeconds)
	{
		return;
	}

	SnapRagdollToTarget(TargetPelvisLoc, TargetPelvisRot, CurrentPelvisLoc, CurrentPelvisRot);
	GroundPenetrationTime = 0.0f;
	++SyncStats.PenetrationRecoveryCount;
}

void UTromboneRagdollComponent::SnapRagdollToTarget(const FVector& TargetPelvisLoc, const FQuat& TargetPelvisRot,
	const FVector& CurrentPelvisLoc, const FQuat& CurrentPelvisRot)
{
	if (!OwnerMesh)
	{
		return;
	}

	/** 골반을 축으로 몸 전체를 한 번에 강체 이동한다. 관절 모양이 그대로라 솔버가 되돌릴 것이 없다.
	 *  SetAllPhysicsRotation은 여기에 못 쓴다. 바디를 제자리에서 돌리기만 하고 옮기지 않아서, 관절이 골반을
	 *  팔다리가 있는 곳으로 다시 끌어당기고, 바닥에서는 팔다리가 항상 이긴다. */
	FQuat DeltaRot = TargetPelvisRot * CurrentPelvisRot.Inverse();
	DeltaRot.Normalize();

	for (FBodyInstance* Body : OwnerMesh->Bodies)
	{
		if (!Body || !Body->IsValidBodyInstance())
		{
			continue;
		}

		const FTransform BodyTransform = Body->GetUnrealWorldTransform();
		const FVector NewLocation = TargetPelvisLoc + DeltaRot.RotateVector(BodyTransform.GetLocation() - CurrentPelvisLoc);
		const FQuat NewRotation = DeltaRot * BodyTransform.GetRotation();
		Body->SetBodyTransform(FTransform(NewRotation, NewLocation, BodyTransform.GetScale3D()), ETeleportType::TeleportPhysics);
	}

	OwnerMesh->SyncComponentToRBPhysics();
}
