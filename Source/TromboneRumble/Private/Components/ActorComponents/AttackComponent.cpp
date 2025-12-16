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
#include "Subsystems/GameStateSubsystem.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	HeadbuttCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HeadbuttCapsuleComponent"));
	HeadbuttCollisionComponent->SetCollisionObjectType(AttackTraceChannel) ; // Object Channel 1 : Weapon
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
	{
		CharacterAnimInstance = Cast<UCharacterAnimInstance>(Mesh->GetAnimInstance());

		if (HeadbuttCollisionComponent)
		{
			HeadbuttCollisionComponent->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadSocketName);
			HeadbuttCollisionComponent->SetRelativeLocation(FVector(0.0f, -20.f, 20.0f));
			HeadbuttCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}

	if (UEquipmentComponent* EquipmentComp = OwnerCharacter->FindComponentByClass<UEquipmentComponent>())
	{
		EquipmentComp->OnEquipmentChangedDelegate.AddDynamic(this, &UAttackComponent::HandleOnEquipmentChanged);
		
		AItemBase* CurrentWeapon = EquipmentComp->GetItemInSlot(EEquipmentSlotType::Instrument);
		HandleOnEquipmentChanged(EEquipmentSlotType::Instrument, CurrentWeapon, nullptr);
	}
}

void UAttackComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsAttacking || !CurrentCollisionComponent || !OwnerCharacter->HasAuthority()) return;

	if (!IsCanSweep()) return;

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
		AttackTraceChannel,
		CapsuleShape,
		Params
	);
	
	if (!bHit) return;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !AlreadyHitActors.Contains(HitActor) && HitActor != OwnerCharacter)
		{
			if (ICombatReceiver* CombatReceiver = Cast<ICombatReceiver>(HitActor))
			{
				AlreadyHitActors.Add(HitActor);

				FHitData HitData;
				HitData.Initiator = OwnerCharacter;
				HitData.HitDirection = (Hit.ImpactPoint - OwnerCharacter->GetActorLocation()).GetSafeNormal();
				HitData.HitType = CurrentAttackData->HitType;

				CombatReceiver->OnHitReceived(HitData);
			}
			
			if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
			{
				FVector KnockbackDir = (HitActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
				KnockbackDir.Z = 0.5f;
				float KnockbackForce = 500.0f;
				
				HitCharacter->LaunchCharacter(KnockbackDir * KnockbackForce, true, true);
			}
		}
	}

	PreviousFrameTransform = CurrentTransform;
}

void UAttackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bIsAttacking);
}

void UAttackComponent::Attack()
{
	if (bIsAttacking || !bCanAttack) return;
	
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
	if (bIsAttacking)
	{
		Client_OnAttackRejected();
		return;
	}
	
	if (!bCanAttack)
	{
		const float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(AttackCooldownTimerHandle);
		if (RemainingTime > AttackCooldownTolerance)
		{
			Client_OnAttackRejected(); 
			return;
		}
        
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
		bCanAttack = true;
	}
	
	StartAttackCooldown();
	SetIsAttacking(true);
	Multicast_PlayAttackEffects();
}

void UAttackComponent::Server_ExecuteAttackEnd_Implementation()
{
	SetIsAttacking(false);
}

void UAttackComponent::Multicast_PlayAttackEffects_Implementation()
{
	if (OwnerCharacter->IsLocallyControlled()) return;

	PlayAttackEffects();
}

void UAttackComponent::Client_OnAttackRejected_Implementation()
{
	if (OwnerCharacter && CharacterAnimInstance)
	{
		OwnerCharacter->StopAnimMontage();
		CharacterAnimInstance->SetIsAttacking(false);
	}

	bIsAttacking = false;
	AlreadyHitActors.Empty();

	GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
	bCanAttack = true;
}

void UAttackComponent::PlayAttackEffects() const
{
	if (CurrentAttackData && CurrentAttackData->AttackAnimMontage)
	{
		CharacterAnimInstance->SetIsAttacking(true);
		OwnerCharacter->PlayAnimMontage(CurrentAttackData->AttackAnimMontage);
	}
}

void UAttackComponent::ResetAttackCooldown()
{
	bCanAttack = true;
}

void UAttackComponent::StartAttackCooldown()
{
	if (!CurrentAttackData) return;
	
	bCanAttack = false;
	
	const float Cooldown = CurrentAttackData->AttackCooldown;
	
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
	if (CurrentAttackData && Montage == CurrentAttackData->AttackAnimMontage)
	{
		Server_ExecuteAttackEnd();
	}
}

void UAttackComponent::HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (Slot != EEquipmentSlotType::Instrument) return;

	GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
	bCanAttack = true;
	
	if (bIsAttacking)
	{
		OwnerCharacter->StopAnimMontage();
		Server_ExecuteAttackEnd();
	}

	if (NewItem)
	{
		if (AInstrumentBase* NewInstrument = Cast<AInstrumentBase>(NewItem))
		{
			CurrentInstrument = NewInstrument;
			CurrentCollisionComponent = NewInstrument->GetCapsuleComponent();
			CurrentAttackData = NewInstrument->GetAttackData();
		}
	}
	else
	{
		CurrentInstrument = nullptr;
		CurrentCollisionComponent = HeadbuttCollisionComponent;
		CurrentAttackData = HeadbuttAttackData;
	}
}

bool UAttackComponent::IsCanSweep() const
{
	const UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return false;

	UGameStateSubsystem* GameStateSubsystem = GI->GetSubsystem<UGameStateSubsystem>();
	if (!GameStateSubsystem || GameStateSubsystem->GetGameState() != EGameState::InGame)
	{
		return false;
	}
	
	return true;
}

void UAttackComponent::SetIsAttacking(const bool bNewIsAttacking)
{
	if (!CharacterAnimInstance) return;

	bIsAttacking = bNewIsAttacking;
	
	if (bNewIsAttacking)
	{
		AlreadyHitActors.Empty();
		PreviousFrameTransform = CurrentCollisionComponent->GetComponentTransform();

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