// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/CharacterAnimInstance.h"
#include "AlphaBlend.h"
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

    bIsStunned = OwnerCharacter->IsStun();

    LocomotionPlayRate = OwnerCharacter->GetLocomotionPlayRate();
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


    const FVector UpVector = OwnerCharacter->GetActorUpVector();
    const FVector Start = OwnerCharacter->GetActorLocation() + UpVector * 10.f;
    const FVector End = Start + UpVector * -200.0f;

    FHitResult HitResult;

    // 자기 자신은 무시
    FCollisionQueryParams Params(SCENE_QUERY_STAT(FootstepTrace), false, OwnerCharacter.Get());
    Params.bReturnPhysicalMaterial = true;

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

    UAkSwitchValue* SwitchValue = NormalFootstepSwitch;

    switch (SurfaceType)
    {
    case SurfaceType_Default:
        SwitchValue = NormalFootstepSwitch;
        break;
    case SurfaceType1:
        SwitchValue = WaterFootstepSwitch ? WaterFootstepSwitch : NormalFootstepSwitch;
        break;
    case SurfaceType2:
        SwitchValue = RockFootStepSwitch ? RockFootStepSwitch : NormalFootstepSwitch;
        break;
    case SurfaceType3:
        SwitchValue = SnowFootStepSwitch ? SnowFootStepSwitch : NormalFootstepSwitch;
        break;
    case SurfaceType4:
        SwitchValue = IceFootStepSwitch ? IceFootStepSwitch : NormalFootstepSwitch;
        break;
    case SurfaceType5:
        SwitchValue = FabricFootStepSwitch ? FabricFootStepSwitch : NormalFootstepSwitch;
        break;
    case SurfaceType6:
        SwitchValue = WoodFootStepSwitch ? WoodFootStepSwitch : NormalFootstepSwitch;
        break;
    default:
        SwitchValue = NormalFootstepSwitch;
        break;
    }

    if (!SwitchValue)
    {
        return;
    }

    OwnerAkSoundComponent->SetSwitch(SwitchValue, FString(TEXT("")), FString(TEXT("")));

    OwnerAkSoundComponent->PostAkEvent(
        FootstepAkEvent,
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
        FAlphaBlendArgs BlendIn;
        BlendIn.BlendTime = 0.0f;
        Montage_PlayWithBlendIn(TargetMontage, BlendIn);
        Montage_SetEndDelegate(EndedDelegate, TargetMontage);
    }
}

void UCharacterAnimInstance::OnGetUpMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == GetUpFrontMontage || Montage == GetUpBackMontage)
    {
        OwnerCharacter->RemoveInputBlock(EInputBlockReason::Ragdoll);
    }
}
