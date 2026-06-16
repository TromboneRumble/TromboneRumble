// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/CapsuleComponent.h"
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

	OwnerCharacter = Cast<ATromboneCharacterBase>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerMesh = OwnerCharacter->GetMesh();
	}
	
	Server_UpdateRagdollTransform();
}

void UTromboneRagdollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
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
	
	DOREPLIFETIME(ThisClass, ServerRagdollState);
}

void UTromboneRagdollComponent::HandleRagdollChanged(bool bIsRagdoll)
{
	if (!OwnerCharacter || !OwnerMesh) return;
	
	if (bEnableDebug)
	{
		const FString DebugMsg = FString::Printf(TEXT("Max Error : %.2f"), PelvisLocationMaxError);
		if (GEngine) GEngine->AddOnScreenDebugMessage(12345, 5.0f, FColor::Red, DebugMsg);
		PelvisLocationMaxError = 0.0f;
		
	}
	
	SetComponentTickEnabled(bIsRagdoll);

	if (bIsRagdoll)
	{
		OwnerCharacter->SetPlayerInput(false);
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
		OwnerMesh->SetSimulatePhysics(true);
		OwnerMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		OwnerMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
	else
	{
		const FVector PelvisLocation = OwnerMesh->GetSocketLocation(PelvisBoneName);
		const FRotator PelvisRotation = OwnerMesh->GetSocketRotation(PelvisBoneName);

		FVector TargetCapsuleLocation = PelvisLocation;
		const FRotator TargetCapsuleRotation = FRotator(0.0f, PelvisRotation.Yaw + 90.0f, 0.0f);

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

		OwnerCharacter->SetActorLocationAndRotation(TargetCapsuleLocation, TargetCapsuleRotation);
		OwnerMesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0.0f, -90.0f, 0.0f));
	}
}

void UTromboneRagdollComponent::Server_UpdateRagdollTransform()
{
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}
	
	FRagdollNetState NewState;
	NewState.PelvisLocation = OwnerMesh->GetBodyInstance(PelvisBoneName)->GetUnrealWorldTransform().GetLocation();
	NewState.PelvisVelocity = OwnerMesh->GetPhysicsLinearVelocity(PelvisBoneName);
	
	ServerRagdollState = NewState;
}

void UTromboneRagdollComponent::Client_InterpolateRagdoll(float DeltaTime)
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

    const FBodyInstance* PelvisBody = OwnerMesh->GetBodyInstance(PelvisBoneName);
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
    const FVector CurrentVelocity = OwnerMesh->GetPhysicsLinearVelocity(PelvisBoneName);
    const FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, VelocityInterpSpeed);

    OwnerMesh->SetPhysicsLinearVelocity(NewVelocity, false, PelvisBoneName);
}

void UTromboneRagdollComponent::OnRep_ServerRagdollState()
{
}


