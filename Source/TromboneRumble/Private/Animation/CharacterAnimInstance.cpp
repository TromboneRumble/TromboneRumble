// Copyright (C) 2026 biksari studio. All Rights Reserved.

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

    OwnerBaseCharacter = Cast<ATromboneCharacterBase>(TryGetPawnOwner());
    OwnerCharacter = Cast<ADefaultTromboneCharacter>(TryGetPawnOwner());
    if (OwnerBaseCharacter.Get())
    {
        MovementComponent = OwnerBaseCharacter->GetCharacterMovement();
        if (OwnerBaseCharacter->GetAkComponent())
        {
			OwnerAkSoundComponent = OwnerBaseCharacter->GetAkComponent();
        }
    }
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    
    const float TargetAlpha = bIsAttacking ? 1.0f : 0.0f;
    
    UpperBodyBlendAlpha = FMath::FInterpTo(UpperBodyBlendAlpha, TargetAlpha, DeltaSeconds, BlendInterpSpeed);

    if (!OwnerBaseCharacter.Get() || !MovementComponent.Get()) return;

    Velocity = MovementComponent->Velocity;

    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
    bIsFalling = MovementComponent->IsFalling();

    const FVector CurrentAcceleration = MovementComponent->GetCurrentAcceleration();
    const bool bIsAccelerating = !CurrentAcceleration.IsNearlyZero();

    bShouldMove = (GroundSpeed > 3.0f) || bIsAccelerating;

    bIsStunned = OwnerBaseCharacter->IsStun();

    // 악기 타입/이동 배속은 플레이어 전용 (NPC는 기본값 유지)
    if (OwnerCharacter.Get())
    {
        CurrentInstrumentType = OwnerCharacter->GetCurrentEquippedInstrumentType();
        LocomotionPlayRate = OwnerCharacter->GetLocomotionPlayRate();
    }
}

void UCharacterAnimInstance::AnimNotify_FootStep()
{
    if (!IsValid(OwnerBaseCharacter) || !OwnerAkSoundComponent.IsValid())
    {
        return;
    }

    UWorld* World = OwnerBaseCharacter->GetWorld();
    if (!World)
    {
        return;
    }


    const FVector UpVector = OwnerBaseCharacter->GetActorUpVector();
    const FVector Start = OwnerBaseCharacter->GetActorLocation() + UpVector * 10.f;
    const FVector End = Start + UpVector * -200.0f;

    FHitResult HitResult;

    // 자기 자신은 무시
    FCollisionQueryParams Params(SCENE_QUERY_STAT(FootstepTrace), false, OwnerBaseCharacter.Get());
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
    if ((Montage == GetUpFrontMontage || Montage == GetUpBackMontage) && OwnerBaseCharacter.Get())
    {
        // A new ragdoll began before this montage ended, so the block belongs to that one.
        if (OwnerBaseCharacter->IsRagdoll()) return;

        OwnerBaseCharacter->RemoveBlock(ECharacterBlockReason::Ragdoll);
        OwnerBaseCharacter->HandleGetUpFinished();
    }
}
