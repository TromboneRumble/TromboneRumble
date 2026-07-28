// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/AttackComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Data/WeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "Items/WeaponBase.h"
#include "TimerManager.h"
#include "Utilities/DebugHelper.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
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
	if (!CurrentWeapon)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("No weapon equipped. Cannot perform attack."));
		return;
	}
	
	if (!CurrentWeapon->CanAttack())
	{
		return;
	}
	
	if (OwnerCharacter->IsLocallyControlled())
	{
		PlayAttackEffects();
	}
	
	Server_ExecuteAttack();
}

void UAttackComponent::Server_ExecuteAttack_Implementation()
{
	if (!CurrentWeapon)
	{
		return;
	}

	if (bAttackInProgress)
	{
		Client_OnAttackRejected();
		return;
	}

	bAttackInProgress = true;
	CurrentWeapon->SetCanAttack(false);
	UpdateAttackDelegateBinding(true);
	Multicast_PlayAttackEffects();

	float FailsafeSeconds = 3.f;
	if (const UAnimMontage* Montage = AttackMontageMap.FindRef(CurrentWeapon->GetWeaponType()))
	{
		FailsafeSeconds = Montage->GetPlayLength() / FMath::Max(Montage->RateScale, UE_KINDA_SMALL_NUMBER);
	}
	constexpr float FailsafeMargin = 0.5f;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ServerAttackFailsafe, this,
		&ThisClass::HandleServerAttackFailsafe, FailsafeSeconds + FailsafeMargin, false);
}

void UAttackComponent::HandleServerAttackFailsafe()
{
	if (!bAttackInProgress)
	{
		return;
	}

	LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Attack failsafe triggered: montage end event was never received. Forcing attack end."));
	Server_ExecuteAttackEnd_Implementation();
}

void UAttackComponent::Server_ExecuteAttackEnd_Implementation()
{
	bAttackInProgress = false;
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_ServerAttackFailsafe);
	}

	if (!CurrentWeapon)
	{
		return;
	}
	
	CurrentWeapon->EndAttack();
	CurrentWeapon->SetCanAttack(true);
	UpdateAttackDelegateBinding(false);
}

void UAttackComponent::Multicast_PlayAttackEffects_Implementation()
{
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	PlayAttackEffects();
}

void UAttackComponent::Client_OnAttackRejected_Implementation()
{
	if (!CurrentWeapon)
	{
		return;
	}

	if (OwnerCharacter && CharacterAnimInstance)
	{
		OwnerCharacter->StopAnimMontage();
		CharacterAnimInstance->SetIsAttacking(false);
	}

	CurrentWeapon->EndAttack();
	CurrentWeapon->SetCanAttack(true);
}

void UAttackComponent::PlayAttackEffects() const
{
	if (!CurrentWeapon)
	{
		return;
	}
	
	const EWeaponType Type = CurrentWeapon->GetWeaponType();
	if (UAnimMontage* MontageToPlay = AttackMontageMap.FindRef(Type))
	{
		CurrentWeapon->SetCanAttack(false);
		CharacterAnimInstance->SetIsAttacking(true);
		
		if (!OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_IsPlaying(MontageToPlay))
		{
			OwnerCharacter->PlayAnimMontage(MontageToPlay);
		}
	}
}

bool UAttackComponent::IsAttackMontage(const UAnimMontage* Montage) const
{
	if (!Montage)
	{
		return false;
	}

	for (const TPair<EWeaponType, TObjectPtr<UAnimMontage>>& Pair : AttackMontageMap)
	{
		if (Pair.Value == Montage)
		{
			return true;
		}
	}
	return false;
}

void UAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!IsAttackMontage(Montage))
	{
		return;
	}

	if (OwnerCharacter)
	{
		Server_ExecuteAttackEnd();
	}
}

void UAttackComponent::HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (Slot != EEquipmentSlotType::Weapon)
	{
		return;
	}
	
	OwnerCharacter->StopAnimMontage();
	CharacterAnimInstance->SetIsAttacking(false);
	
	if (AWeaponBase* OldWeapon = Cast<AWeaponBase>(OldItem))
	{
		OldWeapon->SetCanAttack(true);
		OldWeapon->EndAttack();
	}
	
	Server_ExecuteAttackEnd();

	if (NewItem)
	{
		if (AWeaponBase* NewInstrument = Cast<AWeaponBase>(NewItem))
		{
			CurrentWeapon = NewInstrument;
			CurrentWeapon->SetCanAttack(true);
			CurrentWeapon->EndAttack();
		}
	}
	else
	{
		CurrentWeapon = DefaultWeaponInstance;
	}
}

void UAttackComponent::UpdateAttackDelegateBinding(const bool bIsAttack)
{
	if (!CharacterAnimInstance)
	{
		return;
	}

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