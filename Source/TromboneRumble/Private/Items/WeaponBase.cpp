// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/WeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Data/WeaponDataAsset.h"
#include "Interfaces/CombatReceiver.h"
#include "Subsystems/GameStateSubsystem.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	if (SkeletalMeshComponent)
	{
		SkeletalMeshComponent->SetReceivesDecals(false);
	}
	if (CapsuleComponent)
	{
		CapsuleComponent->SetReceivesDecals(false);
	}
	
	SkeletalMeshComponent->SetCollisionObjectType(AttackTraceChannel); // Object Channel 1 : Weapon
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CapsuleComponent->SetCollisionObjectType(AttackTraceChannel); // Object Channel 1 : Weapon
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void AWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsDetectHit && HasAuthority())
	{
		DetectHit();
	}
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsEquipped);
	DOREPLIFETIME(ThisClass, bCanAttack);
}

bool AWeaponBase::CanInteract_Implementation(AActor* InstigatorActor) const
{
	if (CanBeSwitched)
	{
		return Super::CanInteract_Implementation(InstigatorActor) && !bIsEquipped;
	}
	
	if (const UEquipmentComponent* EquipComp = InstigatorActor->FindComponentByClass<UEquipmentComponent>())
	{
		if (const TObjectPtr<AItemBase> Item = EquipComp->GetItemInSlot(EEquipmentSlotType::Weapon))
		{
			if (const AWeaponBase* Weapon = Cast<AWeaponBase>(Item))
			{
				if (Weapon->GetWeaponType() == EWeaponType::Cymbals ||
					Weapon->GetWeaponType() == EWeaponType::Violin ||
					Weapon->GetWeaponType() == EWeaponType::Trombone)
				{
					return false;
				}
			}
		}
	}
	
	return Super::CanInteract_Implementation(InstigatorActor) && !bIsEquipped;
}

void AWeaponBase::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	//Equip은 EquipComponent를 통해서만
	//Equip(InstigatorActor);
}

void AWeaponBase::Equip(AActor* OwnerActor)
{
	if (!HasAuthority() || bIsEquipped || !OwnerActor) return;
    
	SetOwner(OwnerActor);
	CurrentOwner = OwnerActor;
	bIsEquipped = true;

	OnRep_Equipped();

	if (InteractTriggerComponent) InteractTriggerComponent->SetTriggerActive(false);

	// Move Speed Gameplay Effect 적용
	if (EquipMoveSpeedEffectClass)
	{
		if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OwnerActor))
		{
			if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
			{
				const UGameplayEffect* GE = EquipMoveSpeedEffectClass->GetDefaultObject<UGameplayEffect>();

				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				Context.AddSourceObject(this);

				EquipMoveSpeedEffectHandle =
					ASC->ApplyGameplayEffectToSelf(GE, 1.f, Context);
			}
		}
	}
}

void AWeaponBase::Unequip(AActor* OwnerActor)
{
	if (!HasAuthority() || !bIsEquipped) return;

	if (EquipMoveSpeedEffectHandle.IsValid())
	{
		if (OwnerActor)
		{
			if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OwnerActor))
			{
				if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
				{
					ASC->RemoveActiveGameplayEffect(EquipMoveSpeedEffectHandle);
				}
			}
		}

		EquipMoveSpeedEffectHandle.Invalidate();
	}

	const FVector VForwardImpulse = CurrentOwner->GetActorForwardVector() * WeaponData->WeaponDropForwardImpulse;
	const FVector VUpwardImpulse = FVector::UpVector * WeaponData->WeaponDropUpwardImpulse;

	bIsEquipped = false;
	OnRep_Equipped();
	CurrentOwner = nullptr;
    
	if (InteractTriggerComponent) InteractTriggerComponent->SetTriggerActive(true);
	if (SkeletalMeshComponent) SkeletalMeshComponent->AddImpulse(VForwardImpulse + VUpwardImpulse);
}

void AWeaponBase::DetectHit()
{
	if (!CurrentOwner || !CurrentOwner->HasAuthority() || !IsCanSweep()) return;
	
	UPrimitiveComponent* CollisionComp = GetCollisionComponent();
	if (!CollisionComp) return;

	const FTransform CurrentTransform = CollisionComp->GetComponentTransform();
	const FVector Start = PreviousFrameTransform.GetLocation();
	const FVector End = CurrentTransform.GetLocation();
	const FRotator Rotation = CurrentTransform.GetRotation().Rotator();

	FComponentQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(CurrentOwner);

	TArray<FHitResult> HitResults;

	const bool bHit = GetWorld()->SweepMultiByChannel(
	   HitResults,
	   Start,
	   End,
	   Rotation.Quaternion(),
	   AttackTraceChannel,
	   CollisionComp->GetCollisionShape(),
	   Params
	);
	
	if (!bHit) return;

    for (const FHitResult& Hit : HitResults)
    {
    	AActor* HitActor = Hit.GetActor();
    	if (HitActor && !AlreadyHitActors.Contains(HitActor) && HitActor != CurrentOwner)
    	{
			if (HitActor->Implements<UCombatReceiver>())
			{
				AlreadyHitActors.Add(HitActor);

				FHitData HitData;
				FVector Direction = (Hit.ImpactPoint - CurrentOwner->GetActorLocation()).GetSafeNormal();
				Direction.Z = 0.5f;
				HitData.HitDirection = Direction.GetSafeNormal();
				HitData.KnockbackForce = WeaponData->KnockbackForce;
				HitData.HitType = WeaponData->HitReactionType;

				OnHitSuccess(HitActor);
				ICombatReceiver::Execute_OnHitReceived(HitActor, HitData);
			}
    	}
    }

	PreviousFrameTransform = CurrentTransform;
}

bool AWeaponBase::IsCanSweep() const
{
	const UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return false;

	UGameStateSubsystem* GameStateSubsystem = GI->GetSubsystem<UGameStateSubsystem>();
	if (!GameStateSubsystem || GameStateSubsystem->GetLevelState() != ELevelState::InGame)
	{
		return false;
	}
	
	return true;
}

void AWeaponBase::BeginAttack()
{
	bIsDetectHit = true;
	AlreadyHitActors.Empty();
	SetActorTickEnabled(true); 
	
	if (const UPrimitiveComponent* CollisionComp = GetCollisionComponent())
	{
		PreviousFrameTransform = CollisionComp->GetComponentTransform();
	}
}

void AWeaponBase::EndAttack()
{
	bIsDetectHit = false;
	AlreadyHitActors.Empty();
	SetActorTickEnabled(false); 
}

void AWeaponBase::OnRep_Equipped()
{
	if (bIsEquipped)
	{
		if (!CurrentOwner) return;

		const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner);
		if (!OwnerChar) return;
		
		SetPhysicsEnabled(false);
		AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponData->EquipSocketName);
		SkeletalMeshComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
		SkeletalMeshComponent->IgnoreActorWhenMoving(CurrentOwner, true);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	else
	{
		EndAttack();
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetPhysicsEnabled(true);
		SkeletalMeshComponent->IgnoreActorWhenMoving(CurrentOwner, false);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}