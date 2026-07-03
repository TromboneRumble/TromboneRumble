// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

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
	if (OwnerCharacter)
	{
		OwnerMesh = OwnerCharacter->GetMesh();
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

	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		if (bIsRagdoll)
		{
			if (IsRagdollGrounded())
			{
				RagdollGroundedTime += DeltaTime;
				if (RagdollGroundedTime >= RagdollDuration)
				{
					StopRagdoll();
				}
			}
			else
			{
				RagdollGroundedTime = 0.0f;
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
	DOREPLIFETIME(ThisClass, GetUpLocation);
}

void UTromboneRagdollComponent::StartRagdoll()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || bIsRagdoll)
	{
		return;
	}

	RagdollGroundedTime = 0.0f;
	OwnerCharacter->SetReplicateMovement(false);

	bIsRagdoll = true;
	OnRep_IsRagdoll();

	if (bEnableDebug && bEnableImpulseOnRagdollStart)
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

void UTromboneRagdollComponent::OnRep_IsRagdoll()
{
	if (!OwnerCharacter || !OwnerMesh)
	{
		return;
	}
	
	if (bEnableDebug)
	{
		const FString DebugMsg = FString::Printf(TEXT("Max Distance in Server & Client : %.2f"), PelvisLocationMaxError);
		if (GEngine) GEngine->AddOnScreenDebugMessage(12345, 5.0f, FColor::Red, DebugMsg);
		PelvisLocationMaxError = 0.0f;
	}
	
	SetComponentTickEnabled(bIsRagdoll || bIsBlendingOut);
	
	if (bIsRagdoll)
	{
		bIsBlendingOut = false;
		BlendOutAlpha = 0.0f;

		OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		OwnerMesh->bOnlyAllowAutonomousTickPose = false;

		OwnerMesh->SetSimulatePhysics(true);
		OwnerMesh->SetEnableGravity(true);
		OwnerMesh->SetAllBodiesPhysicsBlendWeight(1.0f);
		OwnerMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		OwnerMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

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

	OnRagdollPhysicsDisabled.Broadcast();
}

bool UTromboneRagdollComponent::IsFacingUp() const
{
	if (!OwnerMesh) return true;

	const FRotator PelvisRotation = OwnerMesh->GetSocketRotation(TromboneBones::Pelvis);
	const FVector PelvisUp = FRotationMatrix(PelvisRotation).GetScaledAxis(EAxis::Z);

	return (FVector::DotProduct(PelvisUp, FVector::UpVector) > 0.0f);
}

bool UTromboneRagdollComponent::IsRagdollGrounded() const
{
	if (!OwnerMesh || !GetWorld()) return false;

	const FVector PelvisLocation = OwnerMesh->GetSocketLocation(TromboneBones::Pelvis);
	const FVector Start = PelvisLocation;
	const FVector End = PelvisLocation - FVector(0.0f, 0.0f, RagdollGroundTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	return GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
}

void UTromboneRagdollComponent::Server_UpdateRagdollTransform()
{
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}
	
	FRagdollNetState NewState;
	NewState.PelvisLocation = OwnerMesh->GetBodyInstance(TromboneBones::Pelvis)->GetUnrealWorldTransform().GetLocation();
	NewState.PelvisVelocity = OwnerMesh->GetPhysicsLinearVelocity(TromboneBones::Pelvis);
	if (const AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		NewState.Timestamp = GameState->GetServerWorldTimeSeconds();
	}
	
	ServerRagdollState = NewState;
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

    const FVector CurrentPelvisLoc = PelvisBody->GetUnrealWorldTransform().GetLocation();

	const UWorld* World = GetWorld();
	float PacketAge = 0.0f;
	if (World && World->GetGameState() && ServerRagdollState.Timestamp > 0.0f)
	{		
		/** GetServerWorldTimeSeconds는 서버 시간을 복제할 때 RTT/2를 보정하지 않으므로, 실제 서버 시간보다 RTT/2만큼 느리다.
		 *  이 RTT/2는 서버에서 날아온 래그돌 패킷의 RTT/2와 동일하므로, 
		 *  'GetServerWorldTimeSeconds - ServerRagdollState.TimeStamp'를 계산할 때 두 RTT/2가 서로 상쇄되어
		 *  패킷이 서버에서 클라까지 날라오는 데 걸린 시간은 계산에서 빠지게 된다.
		 *  이를 보정하기 위해 RTT/2만큼 더해준다.
		 */
		PacketAge = World->GetGameState()->GetServerWorldTimeSeconds() - ServerRagdollState.Timestamp;

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
	if (World && !ServerVelocity.IsNearlyZero(1.0f))
	{
		// 중력가속도(등가속 운동) 반영
		TargetPelvisLoc.Z += 0.5f * World->GetGravityZ() * PacketAge * PacketAge;
	}
	
	if (bEnableDebug && World)
	{
		DrawDebugSphere(World, CurrentPelvisLoc, 10.0f, 8, FColor::Green, false, -1.0f, 0, 1.0f);
		DrawDebugSphere(World, TargetPelvisLoc, 10.0f, 8, FColor::Red, false, -1.0f, 0, 1.0f);
		DrawDebugLine(World, CurrentPelvisLoc, TargetPelvisLoc, FColor::Yellow, false, -1.0f, 0, 1.5f);
		
		const float Distance = FVector::Dist(CurrentPelvisLoc, TargetPelvisLoc);
		PelvisLocationMaxError = std::max(Distance, PelvisLocationMaxError);
		const FString DebugText = FString::Printf(TEXT("Ragdoll difference server and client: %.2f cm"), Distance);
		if (GEngine) GEngine->AddOnScreenDebugMessage(12345, DeltaTime, FColor::Cyan, DebugText);
	}

    if (FVector::DistSquared(CurrentPelvisLoc, TargetPelvisLoc) > ForceLocationUpdateDistance)
    {
        const FVector CurrentRootBodyLoc = OwnerMesh->GetBodyInstance()->GetUnrealWorldTransform().GetLocation();
        const FVector PelvisOffset = CurrentPelvisLoc - CurrentRootBodyLoc;
        OwnerMesh->SetAllPhysicsPosition(TargetPelvisLoc - PelvisOffset);
        return;
    }

    const FVector ToTarget = TargetPelvisLoc - CurrentPelvisLoc;
    const FVector TargetVelocity = ServerVelocity + ToTarget * TrackingIntensity;
    const FVector CurrentVelocity = OwnerMesh->GetPhysicsLinearVelocity(TromboneBones::Pelvis);
    const FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, VelocityInterpSpeed);

    OwnerMesh->SetPhysicsLinearVelocity(NewVelocity, false, TromboneBones::Pelvis);
}



