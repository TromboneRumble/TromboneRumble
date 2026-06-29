// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/InterpolateSpringArmComponent.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/CapsuleComponent.h"

UInterpolateSpringArmComponent::UInterpolateSpringArmComponent()
{
	SetUsingAbsoluteLocation(true);
	SetUsingAbsoluteRotation(true);
	SetIsReplicated(false);
	bDoCollisionTest = false;
	bUsePawnControlRotation = false;
	PrimaryComponentTick.bCanEverTick = true;
}


void UInterpolateSpringArmComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = Cast<ATromboneCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("OwnerCharacter is not ATromboneCharacterBase"));
		return;
	}
}


void UInterpolateSpringArmComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		const FVector TargetLocation = OwnerCharacter->IsRagdoll() ? OwnerCharacter->GetMesh()->GetSocketLocation(TromboneBones::Pelvis) : OwnerCharacter->GetCapsuleComponent()->GetComponentLocation();
		const FVector NewLocation = FMath::VInterpTo(GetComponentLocation(), TargetLocation, DeltaTime, InterpolationSpeed);
		SetWorldLocation(NewLocation);
	}
}

