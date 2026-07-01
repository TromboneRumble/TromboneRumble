// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
        
		OwnerMesh->SetSimulatePhysics(true);
		OwnerMesh->SetEnableGravity(true);
		OwnerMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		OwnerMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		
		if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(OwnerMesh->GetAnimInstance()))
		{
			AnimInst->SetIsRagdoll(true);
		}

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
		OwnerCharacter->SetActorLocationAndRotation(GetUpLocation, TargetCapsuleRotation, false, nullptr, ETeleportType::TeleportPhysics);
		
		const FVector CurrentPelvisLoc = OwnerMesh->GetSocketLocation(TromboneBones::Pelvis);
		FVector DesiredPelvisLoc = CurrentPelvisLoc;
		DesiredPelvisLoc.X = GetUpLocation.X;
		DesiredPelvisLoc.Y = GetUpLocation.Y;

		const FVector CurrentRootBodyLoc = OwnerMesh->GetBodyInstance()->GetUnrealWorldTransform().GetLocation();
		const FVector PelvisOffset = CurrentPelvisLoc - CurrentRootBodyLoc;
		OwnerMesh->SetAllPhysicsPosition(DesiredPelvisLoc - PelvisOffset);

		GetWorld()->GetTimerManager().SetTimerForNextTick(
		   FTimerDelegate::CreateUObject(this, &ThisClass::SavePoseSnapshot)
		);

		OnRagdollEnded.Broadcast();
	}
}

void UTromboneRagdollComponent::SavePoseSnapshot()
{
	if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(OwnerMesh->GetAnimInstance()))
	{
		AnimInst->SaveRagdollPoseSnapshot();
		AnimInst->PlayGetUpMontage(IsFacingUp());
	}
	
	BeginRagdollBlendOut();
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

	const float EasedAlpha = RagdollBlendOutCurve
		? RagdollBlendOutCurve->GetFloatValue(BlendOutAlpha)
		: BlendOutAlpha;

	const float PhysicsWeight = FMath::Lerp(1.0f, 0.0f, EasedAlpha);
	OwnerMesh->SetAllBodiesPhysicsBlendWeight(PhysicsWeight);

	if (bEnableDebug && GEngine)
	{
		const FString DebugMsg = FString::Printf(TEXT("Ragdoll blend-out: alpha=%.2f weight=%.2f"), BlendOutAlpha, PhysicsWeight);
		GEngine->AddOnScreenDebugMessage(12346, DeltaTime, FColor::Orange, DebugMsg);
	}

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
    const FVector TargetPelvisLoc = ServerRagdollState.PelvisLocation;
	
	if (bEnableDebug && GetWorld())
	{
		DrawDebugSphere(GetWorld(), CurrentPelvisLoc, 10.0f, 8, FColor::Green, false, -1.0f, 0, 1.0f);
		DrawDebugSphere(GetWorld(), TargetPelvisLoc, 10.0f, 8, FColor::Red, false, -1.0f, 0, 1.0f);
		DrawDebugLine(GetWorld(), CurrentPelvisLoc, TargetPelvisLoc, FColor::Yellow, false, -1.0f, 0, 1.5f);
		
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
    const FVector TargetVelocity = ServerRagdollState.PelvisVelocity + ToTarget * TrackingIntensity;
    const FVector CurrentVelocity = OwnerMesh->GetPhysicsLinearVelocity(TromboneBones::Pelvis);
    const FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, VelocityInterpSpeed);

    OwnerMesh->SetPhysicsLinearVelocity(NewVelocity, false, TromboneBones::Pelvis);
}

void UTromboneRagdollComponent::OnRep_ServerRagdollState()
{
	// Don't have anything to do right now.
}


