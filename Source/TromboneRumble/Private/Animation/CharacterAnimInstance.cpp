// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/CharacterAnimInstance.h"

#include "Utilities/DebugHelper.h"

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    
    const float TargetAlpha = bIsAttacking ? 1.0f : 0.0f;
    
    UpperBodyBlendAlpha = FMath::FInterpTo(UpperBodyBlendAlpha, TargetAlpha, DeltaSeconds, BlendInterpSpeed);
}

void UCharacterAnimInstance::SetIsAttacking(const bool bNewIsAttacking)
{
    bIsAttacking = bNewIsAttacking;
}
