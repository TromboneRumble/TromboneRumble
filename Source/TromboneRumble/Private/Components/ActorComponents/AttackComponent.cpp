// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/AttackComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/AttackDataAsset.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "Utilities/DebugHelper.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAttackComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsAttacking || !CollisionComponent || !OwnerCharacter->HasAuthority()) return;

	const FTransform CurrentTransform = CollisionComponent->GetComponentTransform();
	const FVector Start = PreviousFrameTransform.GetLocation();
	const FVector End = CurrentTransform.GetLocation();
	const FRotator Rotation = CurrentTransform.GetRotation().Rotator();
    
	TArray<FHitResult> HitResults;
	FComponentQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(CollisionComponent->GetOwner());
	
	const FCollisionShape CapsuleShape = CollisionComponent->GetCollisionShape();
	
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
	if (!OwnerCharacter || (OwnerCharacter->GetLocalRole() < ROLE_AutonomousProxy)) return;

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
	if (bIsAttacking || !bCanAttack || !CollisionComponent || !CurrentAttackData) return;

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
	PreviousFrameTransform = CollisionComponent->GetComponentTransform();

	Multicast_PlayAttackEffects();
}

void UAttackComponent::Server_ExecuteAttackEnd_Implementation()
{
	bIsAttacking = false;
	AlreadyHitActors.Empty();
}

void UAttackComponent::Multicast_PlayAttackEffects_Implementation()
{
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled() && !OwnerCharacter->HasAuthority()) return;
	
	if (!OwnerCharacter || !CurrentAttackData || !CurrentAttackData->AttackAnimMontage) return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	if (OwnerCharacter->HasAuthority())
	{
		if (!AnimInstance->OnMontageEnded.IsAlreadyBound(this, &ThisClass::OnAttackMontageEnded))
		{
			AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::OnAttackMontageEnded);
		}
	}
	
	OwnerCharacter->PlayAnimMontage(CurrentAttackData->AttackAnimMontage);
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