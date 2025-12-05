// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/CharacterAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

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
