// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/InstrumentAttackComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Utilities/DebugHelper.h"

UInstrumentAttackComponent::UInstrumentAttackComponent()
{
	
}

void UInstrumentAttackComponent::Attack()
{
	if (bIsAttacking || !InstrumentCollisionComponent || !OwnerCharacter) return;
	
	if (OwnerCharacter->HasAuthority())
	{
		Server_ExecuteAttack_Implementation();
	}
	else
	{
		Server_ExecuteAttack();
	}
}

void UInstrumentAttackComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsAttacking || !InstrumentCollisionComponent || !OwnerCharacter->HasAuthority()) return;

	const FTransform CurrentTransform = InstrumentCollisionComponent->GetComponentTransform();
	const FVector Start = PreviousFrameTransform.GetLocation();
	const FVector End = CurrentTransform.GetLocation();
	const FRotator Rotation = CurrentTransform.GetRotation().Rotator();
    
	TArray<FHitResult> HitResults;
	FComponentQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(InstrumentCollisionComponent->GetOwner());
	
	const FCollisionShape CapsuleShape = InstrumentCollisionComponent->GetCollisionShape();
	
	const bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		Rotation.Quaternion(),
		ECC_GameTraceChannel1,
		CapsuleShape,
		Params
	);

	// TODO : Remove debug drawing
	if (const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(InstrumentCollisionComponent))
	{
		float Radius, HalfHeight;
		CapsuleComp->GetUnscaledCapsuleSize(Radius, HalfHeight);
		DrawDebugCapsule(GetWorld(), Start, HalfHeight, Radius, Rotation.Quaternion(), FColor::Red, false, 0.5f);
		DrawDebugCapsule(GetWorld(), End, HalfHeight, Radius, Rotation.Quaternion(), FColor::Yellow, false, 0.5f);
	}
	
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
				HitData.HitType = EHitType::Instrument;

				CombatReceiver->OnHitReceived(HitData);
			}
		}
	}

	PreviousFrameTransform = CurrentTransform;
}

void UInstrumentAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == InstrumentAttackAnimMontage)
	{
		Server_ExecuteEndAttack_Implementation();

		if (OwnerCharacter)
		{
			if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
			{
				AnimInstance->OnMontageEnded.RemoveDynamic(this, &UInstrumentAttackComponent::OnAttackMontageEnded);
			}
		}
	}
}

void UInstrumentAttackComponent::Server_ExecuteAttack_Implementation()
{
	if (bIsAttacking) return;
	
	bIsAttacking = true;
	AlreadyHitActors.Empty();
	PreviousFrameTransform = InstrumentCollisionComponent->GetComponentTransform();

	Multicast_PlayAttackEffects();
}

void UInstrumentAttackComponent::Multicast_PlayAttackEffects_Implementation()
{
	if (OwnerCharacter && InstrumentAttackAnimMontage)
	{
		if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			if (OwnerCharacter->HasAuthority())
			{
				if (!AnimInstance->OnMontageEnded.IsAlreadyBound(this, &UInstrumentAttackComponent::OnAttackMontageEnded))
				{
					AnimInstance->OnMontageEnded.AddDynamic(this, &UInstrumentAttackComponent::OnAttackMontageEnded);
				}
			}
		}
		OwnerCharacter->PlayAnimMontage(InstrumentAttackAnimMontage);
	}
}

void UInstrumentAttackComponent::Server_ExecuteEndAttack_Implementation()
{
	bIsAttacking = false;
	AlreadyHitActors.Empty();
}