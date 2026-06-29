// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/DebugHelper.h"

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
	
	Server_UpdateRagdollTransform();
}

void UTromboneRagdollComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

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
	else
	{
		Client_InterpolateRagdoll(DeltaTime);
	}
}

void UTromboneRagdollComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsRagdoll);
	DOREPLIFETIME(ThisClass, ServerRagdollState);
}

void UTromboneRagdollComponent::StartRagdoll()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || bIsRagdoll)
	{
		return;
	}

	bIsRagdoll = true;
	OnRep_IsRagdoll();
	
	RagdollGroundedTime = 0.0f;

	if (bEnableDebug && bEnableImpulseOnRagdollStart)
	{
		constexpr float UpForce = 5000.f;
		constexpr float RandomRange = 1500.f;

		const float RandomX = FMath::FRandRange(-RandomRange, RandomRange);
		const float RandomY = FMath::FRandRange(-RandomRange, RandomRange);

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

	bIsRagdoll = false;
	OnRep_IsRagdoll();
}

void UTromboneRagdollComponent::OnRep_IsRagdoll()
{
	if (!OwnerCharacter || !OwnerMesh)
	{
		return;
	}
	
	if (bEnableDebug)
	{
		const FString DebugMsg = FString::Printf(TEXT("Max Error : %.2f"), PelvisLocationMaxError);
		if (GEngine) GEngine->AddOnScreenDebugMessage(12345, 5.0f, FColor::Red, DebugMsg);
		PelvisLocationMaxError = 0.0f;
	}
	
	SetComponentTickEnabled(bIsRagdoll);
	
	if (bIsRagdoll)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
		OwnerMesh->SetSimulatePhysics(true);
		OwnerMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		OwnerMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		
		if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(OwnerMesh->GetAnimInstance()))
		{
			AnimInst->SetIsRagdolling(true);
		}

		OnRagdollStarted.Broadcast();
	}
	else
	{
		const FVector PelvisLocation = OwnerMesh->GetSocketLocation(TromboneBones::Pelvis);
		const FRotator PelvisRotation = OwnerMesh->GetSocketRotation(TromboneBones::Pelvis);

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
			PRINT_WITH_CURRENT_CONTEXT(TEXT("Warning: Failed to find ground for capsule placement after ragdoll. Using pelvis location as fallback."));
		}

		OwnerCharacter->SetActorLocation(TargetCapsuleLocation);
		OwnerMesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0.0f, -90.0f, 0.0f));
		
		GetWorld()->GetTimerManager().SetTimerForNextTick(
		   FTimerDelegate::CreateUObject(this, &ThisClass::DelayedSavePoseSnapshot)
		);
		
		OnRagdollEnded.Broadcast();
	}
}

void UTromboneRagdollComponent::DelayedSavePoseSnapshot()
{
	if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(OwnerMesh->GetAnimInstance()))
	{
		AnimInst->SaveRagdollPoseSnapshot();
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick(
	   FTimerDelegate::CreateUObject(this, &ThisClass::UnapplyRagdoll)
	);
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

	if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(OwnerMesh->GetAnimInstance()))
	{
		AnimInst->PlayGetUpMontage(IsFacingUp());
	}

	// TODO : getup 브로드캐스팅 타이밍 명확하게
	OnRagdollGetUp.Broadcast();
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

void UTromboneRagdollComponent::Client_InterpolateRagdoll(const float DeltaTime)
{
    if (!OwnerMesh)
    {
	    UE_LOG(LogTemp, Warning, TEXT("[RagdollComponent::Client_InterpolateRagdoll] SkeletalMeshComponent is NULL"));
    	return;
    }
	
	if (!OwnerCharacter)
    {
		UE_LOG(LogTemp, Warning, TEXT("[RagdollComponent::Client_InterpolateRagdoll] Character is NULL"));
    	return;
    }

    const FBodyInstance* PelvisBody = OwnerMesh->GetBodyInstance(TromboneBones::Pelvis);
    if (!PelvisBody)
    {
	    UE_LOG(LogTemp, Warning, TEXT("[RagdollComponent::Client_InterpolateRagdoll] Pelvis BodyInstance is NULL"));
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
}


