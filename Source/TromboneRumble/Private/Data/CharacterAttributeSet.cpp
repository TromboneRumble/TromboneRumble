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
