// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/CharacterAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/DebugHelper.h"

void UCharacterAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(
        UCharacterAttributeSet,
        MoveSpeed,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UCharacterAttributeSet,
        GroundFriction,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UCharacterAttributeSet,
        BrakingDeceleration,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UCharacterAttributeSet,
        LocomotionPlayRate,
        COND_None,
        REPNOTIFY_Always
    );
}

void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
    {
        const float NewValue = MoveSpeed.GetCurrentValue();

        Debug::Print(
            FString::Printf(
                TEXT("[MoveSpeed Execute] NewValue: %.1f (SourceGE: %s)"),
                NewValue,
                *GetNameSafe(Data.EffectSpec.Def)
            )
        );
    }
}

void UCharacterAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
    if (Attribute == GetMoveSpeedAttribute())
    {
        if (AActor* Owner = GetOwningActor())
        {
            if (ACharacter* Character = Cast<ACharacter>(Owner))
            {
                if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
                {
                    Move->MaxWalkSpeed = NewValue;
                }
            }
        }
    }
    else if (Attribute == GetGroundFrictionAttribute())
    {
        if (AActor* Owner = GetOwningActor())
        {
            if (ACharacter* Character = Cast<ACharacter>(Owner))
            {
                if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
                {
                    Move->GroundFriction = NewValue;
                }
            }
        }
    }
    else if (Attribute == GetBrakingDecelerationAttribute())
    {
        if (AActor* Owner = GetOwningActor())
        {
            if (ACharacter* Character = Cast<ACharacter>(Owner))
            {
                if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
                {
                    Move->BrakingDecelerationWalking = NewValue;
                }
            }
        }
    }
}

void UCharacterAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, MoveSpeed, OldValue);
    if (AActor* Owner = GetOwningActor())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
            {
                Move->MaxWalkSpeed = GetMoveSpeed();
            }
        }
    }
}

void UCharacterAttributeSet::OnRep_GroundFriction(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, GroundFriction, OldValue);
    if (AActor* Owner = GetOwningActor())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
            {
                Move->GroundFriction = GetGroundFriction();
            }
        }
    }
}

void UCharacterAttributeSet::OnRep_BrakingDeceleration(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, BrakingDeceleration, OldValue);
    if (AActor* Owner = GetOwningActor())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
            {
                Move->BrakingDecelerationWalking = GetBrakingDeceleration();
            }
        }
    }
}

void UCharacterAttributeSet::OnRep_LocomotionPlayRate(const FGameplayAttributeData& OldValue)
{
    // CMC 반영 없음 — AnimInstance 가 매 프레임 GetLocomotionPlayRate() 로 읽어 로코모션 Play Rate 에 사용
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, LocomotionPlayRate, OldValue);
}
