// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/InstrumentAttackComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"

UInstrumentAttackComponent::UInstrumentAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInstrumentAttackComponent::Attack()
{
	if (!InstrumentMesh || !OwnerCharacter) return;
	
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

	if (!bIsAttacking || !InstrumentMesh || !OwnerCharacter->HasAuthority()) return;

	const FTransform CurrentTransform = InstrumentMesh->GetComponentTransform();
	const FVector Start = PreviousFrameTransform.GetLocation();
	const FVector End = CurrentTransform.GetLocation();
	const FRotator Rotation = CurrentTransform.GetRotation().Rotator();
    
	TArray<FHitResult> HitResults;
	FComponentQueryParams Params;
	
	const bool bHit = GetWorld()->ComponentSweepMulti(HitResults, InstrumentMesh, Start, End, Rotation, Params);
	if (!bHit) return;
	
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !AlreadyHitActors.Contains(HitActor) && HitActor != OwnerCharacter && HitActor->Implements<UCombatReceiver>())
		{
			if (ICombatReceiver* CombatReceiver = Cast<ICombatReceiver>(HitActor))
			{
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

void UInstrumentAttackComponent::Server_ExecuteAttack_Implementation()
{
	bIsAttacking = true;
	AlreadyHitActors.Empty();
	PreviousFrameTransform = InstrumentMesh->GetComponentTransform();

	Multicast_PlayAttackEffects();
}

void UInstrumentAttackComponent::Multicast_PlayAttackEffects_Implementation()
{
	if (OwnerCharacter && InstrumentAttackAnimMontage)
	{
		OwnerCharacter->PlayAnimMontage(InstrumentAttackAnimMontage);
	}
}

void UInstrumentAttackComponent::Server_ExecuteEndAttack_Implementation()
{
	bIsAttacking = false;
	AlreadyHitActors.Empty();
}