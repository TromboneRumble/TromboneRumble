// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/AttackComponent.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Data/AttackDataAsset.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "Items/InstrumentBase.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/DebugHelper.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	HeadbuttCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HeadbuttCapsuleComponent"));
	HeadbuttCollisionComponent->SetCollisionObjectType(ECC_GameTraceChannel1) ; // Object Channel 1 : Weapon
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->GetMesh() && HeadbuttCollisionComponent)
	{
		HeadbuttCollisionComponent->AttachToComponent(
			OwnerCharacter->GetMesh(), 
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
			FName("head")
		);
		HeadbuttCollisionComponent->Activate(); 
		HeadbuttCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		
		CharacterAnimInstance = Cast<UCharacterAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance());

		UEquipmentComponent* EquipmentComp = OwnerCharacter->FindComponentByClass<UEquipmentComponent>();
		if (IsValid(EquipmentComp))
		{
			EquipmentComp->OnEquipmentChangedDelegate.AddDynamic(this, &UAttackComponent::HandleOnEquipmentChanged);
		
			AItemBase* CurrentWeapon = EquipmentComp->GetItemInSlot(EEquipmentSlotType::Instrument);
			HandleOnEquipmentChanged(EEquipmentSlotType::Instrument, CurrentWeapon, nullptr);
		}
	}
}

void UAttackComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsAttacking || !CurrentCollisionComponent || !OwnerCharacter->HasAuthority()) return;

	const FTransform CurrentTransform = CurrentCollisionComponent->GetComponentTransform();
	const FVector Start = PreviousFrameTransform.GetLocation();
	const FVector End = CurrentTransform.GetLocation();
	const FRotator Rotation = CurrentTransform.GetRotation().Rotator();
    
	TArray<FHitResult> HitResults;
	FComponentQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(CurrentCollisionComponent->GetOwner());
	
	const FCollisionShape CapsuleShape = CurrentCollisionComponent->GetCollisionShape();
	
	const bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		Rotation.Quaternion(),
		ECC_GameTraceChannel1,
		CapsuleShape,
		Params
	);
	
	if (!bHit) return;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !AlreadyHitActors.Contains(HitActor) && HitActor != OwnerCharacter && HitActor->Implements<UCombatReceiver>())
		{
			if (ICombatReceiver* CombatReceiver = Cast<ICombatReceiver>(HitActor))
			{
				PRINT_WITH_CURRENT_CONTEXT("Instrument hit actor: " + (HitActor ? HitActor->GetName() : TEXT("None")));
				AlreadyHitActors.Add(HitActor);

				FHitData HitData;
				HitData.Initiator = OwnerCharacter;
				HitData.HitDirection = (Hit.ImpactPoint - OwnerCharacter->GetActorLocation()).GetSafeNormal();
				HitData.HitType = CurrentAttackData->HitType;

				CombatReceiver->OnHitReceived(HitData);
			}
		}
	}

	PreviousFrameTransform = CurrentTransform;
}

void UAttackComponent::Attack()
{
	if (!OwnerCharacter || !CharacterAnimInstance) return;
	
	if (OwnerCharacter->GetLocalRole() < ROLE_AutonomousProxy) return;

	if (bIsAttacking || !bCanAttack || !CurrentAttackData) return;
	
	if (OwnerCharacter->IsLocallyControlled() && !OwnerCharacter->HasAuthority())
	{
		bCanAttack = false;
		GetWorld()->GetTimerManager().SetTimer(
		   AttackCooldownTimerHandle,
		   this,
		   &ThisClass::ResetAttackCooldown,
		   CurrentAttackData->AttackCooldown,
		   false
		);
		if (CurrentAttackData->AttackAnimMontage)
		{
			CharacterAnimInstance->SetIsAttacking(true);
			OwnerCharacter->PlayAnimMontage(CurrentAttackData->AttackAnimMontage);
		}
	}

	if (OwnerCharacter->HasAuthority())
	{
		Server_ExecuteAttack_Implementation();
	}
	else
	{
		Server_ExecuteAttack();
	}
}

void UAttackComponent::Server_ExecuteAttack_Implementation()
{
	if (bIsAttacking || !bCanAttack || !CurrentCollisionComponent || !CurrentAttackData) return;

	bCanAttack = false;
	GetWorld()->GetTimerManager().SetTimer(
		AttackCooldownTimerHandle,
		this,
		&ThisClass::ResetAttackCooldown,
		CurrentAttackData->AttackCooldown,
		false
	);
	
	bIsAttacking = true;
	AlreadyHitActors.Empty();
	PreviousFrameTransform = CurrentCollisionComponent->GetComponentTransform();

	Multicast_PlayAttackEffects();
}

void UAttackComponent::Server_ExecuteAttackEnd_Implementation()
{
	bIsAttacking = false;
	AlreadyHitActors.Empty();

	Multicast_ExecuteAttackEnd();
}

void UAttackComponent::Multicast_PlayAttackEffects_Implementation()
{
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled() && !OwnerCharacter->HasAuthority()) return;
	
	if (!OwnerCharacter || !CharacterAnimInstance || !CurrentAttackData || !CurrentAttackData->AttackAnimMontage) return;

	if (const AInstrumentBase* Instrument = FindEquippedInstrument())
	{
		Instrument->AttachToAttackSocket();
	}

	if (OwnerCharacter->HasAuthority())
	{
		if (!CharacterAnimInstance->OnMontageEnded.IsAlreadyBound(this, &ThisClass::OnAttackMontageEnded))
		{
			CharacterAnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::OnAttackMontageEnded);
		}
	}

	CharacterAnimInstance->SetIsAttacking(true);
	OwnerCharacter->PlayAnimMontage(CurrentAttackData->AttackAnimMontage);
}

void UAttackComponent::Multicast_ExecuteAttackEnd_Implementation()
{
	if (CharacterAnimInstance)
	{
		CharacterAnimInstance->SetIsAttacking(false);
	}

	if (const AInstrumentBase* Instrument = FindEquippedInstrument())
	{
		Instrument->AttachToIdleSocket();
	}
}

void UAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CurrentAttackData && Montage == CurrentAttackData->AttackAnimMontage)
	{
		Server_ExecuteAttackEnd_Implementation();

		if (OwnerCharacter)
		{
			if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
			{
				AnimInstance->OnMontageEnded.RemoveDynamic(this, &ThisClass::OnAttackMontageEnded);
			}
		}
	}
}

void UAttackComponent::HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (Slot != EEquipmentSlotType::Instrument) return;

	if (bIsAttacking)
	{
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Stop(0.1f);

		if (OwnerCharacter->HasAuthority())
		{
			Server_ExecuteAttackEnd_Implementation();
		}
		else
		{
			Server_ExecuteAttackEnd(); 
		}
	}

	if (NewItem)
	{
		if (const AInstrumentBase* NewInstrument = Cast<AInstrumentBase>(NewItem))
		{
			CurrentCollisionComponent = NewInstrument->GetCapsuleComponent();
			CurrentAttackData = NewInstrument->GetAttackData();
		}
	}
	else
	{
		CurrentCollisionComponent = HeadbuttCollisionComponent;
		CurrentAttackData = HeadbuttAttackData;
	}
}

AInstrumentBase* UAttackComponent::FindEquippedInstrument() const
{
	if (!OwnerCharacter) return nullptr;

	TArray<AActor*> AttachedActors;
	OwnerCharacter->GetAttachedActors(AttachedActors);

	for (AActor* AttachedActor : AttachedActors)
	{
		if (AInstrumentBase* Instrument = Cast<AInstrumentBase>(AttachedActor))
		{
			return Instrument;
		}
	}

	return nullptr;
}