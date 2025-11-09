// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/CharacterAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    
    const float TargetAlpha = bIsAttacking ? 1.0f : 0.0f;
    
    UpperBodyBlendAlpha = FMath::FInterpTo(UpperBodyBlendAlpha, TargetAlpha, DeltaSeconds, BlendInterpSpeed);

    if (!OwnerCharacter || !MovementComponent) return;

    Velocity = MovementComponent->Velocity;
    
    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
    bIsFalling = MovementComponent->IsFalling();
    
    const FVector CurrentAcceleration = MovementComponent->GetCurrentAcceleration();
    const bool bIsAccelerating = !CurrentAcceleration.IsNearlyZero();
    
    bShouldMove = (GroundSpeed > 3.0f) || bIsAccelerating;
}

void UCharacterAnimInstance::SetIsAttacking(const bool bNewIsAttacking)
{
    bIsAttacking = bNewIsAttacking;
}
