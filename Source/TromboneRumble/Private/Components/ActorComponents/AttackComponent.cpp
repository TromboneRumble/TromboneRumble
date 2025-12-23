// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/AttackComponent.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Data/WeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "Items/WeaponBase.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
	{
		CharacterAnimInstance = Cast<UCharacterAnimInstance>(Mesh->GetAnimInstance());
	}

	if (UEquipmentComponent* EquipmentComp = OwnerCharacter->FindComponentByClass<UEquipmentComponent>())
	{
		EquipmentComp->OnEquipmentChangedDelegate.AddDynamic(this, &UAttackComponent::HandleOnEquipmentChanged);
		
		AItemBase* CurrentEquippedWeapon = EquipmentComp->GetItemInSlot(EEquipmentSlotType::Weapon);
		HandleOnEquipmentChanged(EEquipmentSlotType::Weapon, CurrentEquippedWeapon, nullptr);
	}
}

void UAttackComponent::Attack()
{
	if (!CurrentWeapon) return;
	
	if (CurrentWeapon->GetIsAttacking() || !CurrentWeapon->IsCanAttack()) return;
	
	if (!OwnerCharacter->HasAuthority())
	{
		StartAttackCooldown();
	}
	
	if (OwnerCharacter->IsLocallyControlled())
	{
		PlayAttackEffects();
	}

	Server_ExecuteAttack();
}

void UAttackComponent::Server_ExecuteAttack_Implementation()
{
	if (!CurrentWeapon) return;

	if (CurrentWeapon->GetIsAttacking())
	{
		Client_OnAttackRejected();
		return;
	}
	
	if (!CurrentWeapon->IsCanAttack())
	{
		const float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(AttackCooldownTimerHandle);
		if (RemainingTime > AttackCooldownTolerance)
		{
			Client_OnAttackRejected(); 
			return;
		}
        
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
		CurrentWeapon->SetCanAttack(true);
	}
	
	CurrentWeapon->BeginAttack();
	StartAttackCooldown();
	UpdateAttackDelegateBinding(true);
	Multicast_PlayAttackEffects();
}

void UAttackComponent::Server_ExecuteAttackEnd_Implementation()
{
	if (!CurrentWeapon) return;
	
	CurrentWeapon->EndAttack();
	UpdateAttackDelegateBinding(false);
}

void UAttackComponent::Multicast_PlayAttackEffects_Implementation()
{
	if (OwnerCharacter->IsLocallyControlled()) return;

	PlayAttackEffects();
}

void UAttackComponent::Client_OnAttackRejected_Implementation()
{
	if (!CurrentWeapon) return;

	if (OwnerCharacter && CharacterAnimInstance)
	{
		OwnerCharacter->StopAnimMontage();
		CharacterAnimInstance->SetIsAttacking(false);
	}

	CurrentWeapon->EndAttack();
	GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
}

void UAttackComponent::PlayAttackEffects() const
{
	if (!CurrentWeapon) return;
	
	const EWeaponType Type = CurrentWeapon->GetWeaponType();
	if (UAnimMontage* MontageToPlay = AttackMontageMap.FindRef(Type))
	{
		OwnerCharacter->PlayAnimMontage(MontageToPlay);
		CharacterAnimInstance->SetIsAttacking(true);
	}
}

void UAttackComponent::ResetAttackCooldown()
{
	if (!CurrentWeapon) return;

	CurrentWeapon->SetCanAttack(true);
}

void UAttackComponent::StartAttackCooldown()
{
	if (!CurrentWeapon) return;

	const float Cooldown = CurrentWeapon->GetAttackCooldown();
	
	if (Cooldown <= 0.0f)
	{
		ResetAttackCooldown();
		return;
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		AttackCooldownTimerHandle,
		this,
		&ThisClass::ResetAttackCooldown,
		Cooldown,
		false
	);
}

void UAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	Server_ExecuteAttackEnd();
}

void UAttackComponent::HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (Slot != EEquipmentSlotType::Weapon) return;

	GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
	
	OwnerCharacter->StopAnimMontage();
	Server_ExecuteAttackEnd();

	if (NewItem)
	{
		if (AWeaponBase* NewInstrument = Cast<AWeaponBase>(NewItem))
		{
			CurrentWeapon = NewInstrument;
		}
	}
	else
	{
		CurrentWeapon = DefaultWeaponInstance;
	}
}

void UAttackComponent::UpdateAttackDelegateBinding(const bool bIsAttack)
{
	if (!CharacterAnimInstance) return;

	if (bIsAttack)
	{
		if (!CharacterAnimInstance->OnMontageEnded.IsAlreadyBound(this, &ThisClass::OnAttackMontageEnded))
		{
			CharacterAnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::OnAttackMontageEnded);
		}
	}
	else
	{
		CharacterAnimInstance->OnMontageEnded.RemoveDynamic(this, &ThisClass::OnAttackMontageEnded);
	}
}