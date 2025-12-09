// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/CharacterAnimInstance.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AkComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ADefaultTromboneCharacter>(TryGetPawnOwner());
    if (OwnerCharacter.Get())
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        if (OwnerCharacter->GetAkComponent())
        {
			OwnerAkSoundComponent = OwnerCharacter->GetAkComponent();
        }
    }
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    
    const float TargetAlpha = bIsAttacking ? 1.0f : 0.0f;
    
    UpperBodyBlendAlpha = FMath::FInterpTo(UpperBodyBlendAlpha, TargetAlpha, DeltaSeconds, BlendInterpSpeed);

    if (!OwnerCharacter.Get() || !MovementComponent.Get()) return;

    Velocity = MovementComponent->Velocity;
    
    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
    bIsFalling = MovementComponent->IsFalling();
    
    const FVector CurrentAcceleration = MovementComponent->GetCurrentAcceleration();
    const bool bIsAccelerating = !CurrentAcceleration.IsNearlyZero();
    
    bShouldMove = (GroundSpeed > 3.0f) || bIsAccelerating;

    CurrentInstrumentType = OwnerCharacter->GetCurrentEquippedInstrumentType();
}

void UCharacterAnimInstance::AnimNotify_FootStep()
{
    if (!IsValid(OwnerCharacter) || !OwnerAkSoundComponent.IsValid())
    {
        return;
    }

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        return;
    }

   
    const FVector Start = OwnerCharacter->GetActorLocation();

    const FVector UpVector = OwnerCharacter->GetActorUpVector();
    const FVector End = Start + UpVector * -200.0f;

    FHitResult HitResult;

    // 자기 자신은 무시
    FCollisionQueryParams Params(SCENE_QUERY_STAT(FootstepTrace), false, OwnerCharacter.Get());

    const bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        Params
    );

    if (!bHit)
    {
        return;
    }

    
    const EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(HitResult);

    UAkAudioEvent* EventToPost = FootstepAkEvent;

    switch (SurfaceType)
    {
    case SurfaceType_Default:
        EventToPost = FootstepAkEvent;
        break;
    case SurfaceType1:
        EventToPost = FootstepWaterAkEvent ? FootstepWaterAkEvent : FootstepAkEvent;
        break;
    default:
        EventToPost = FootstepAkEvent;
        break;
    }

    if (!EventToPost)
    {
        return;
    }

    OwnerAkSoundComponent->PostAkEvent(
        EventToPost,
        0,                                  
        FOnAkPostEventCallback() 
    );
}

void UCharacterAnimInstance::PlayGetUpMontage(const bool bIsFacingUp)
{
    FOnMontageEnded EndedDelegate;
    EndedDelegate.BindUObject(this, &UCharacterAnimInstance::OnGetUpMontageEnded);

    if (UAnimMontage* TargetMontage = bIsFacingUp ? GetUpBackMontage : GetUpFrontMontage)
    {
        Montage_Play(TargetMontage);
        Montage_SetEndDelegate(EndedDelegate, TargetMontage);
    }
}

void UCharacterAnimInstance::OnGetUpMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == GetUpFrontMontage || Montage == GetUpBackMontage)
    {
        OwnerCharacter->EnablePlayerInput();
    }
}
