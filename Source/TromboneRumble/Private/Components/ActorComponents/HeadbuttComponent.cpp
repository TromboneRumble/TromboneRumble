// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/HeadbuttComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "Kismet/KismetSystemLibrary.h"

void UHeadbuttComponent::Attack()
{
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		Server_ExecuteAttack_Implementation();
	}
	else
	{
		Server_ExecuteAttack();
	}
}

void UHeadbuttComponent::Server_ExecuteAttack_Implementation()
{
	if (!OwnerCharacter || !OwnerCharacter->GetMesh()) return;

	const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	const FName HeadSocketName = TEXT("socket_head");
	const FVector Start = Mesh->GetSocketLocation(HeadSocketName);
	const FVector End = Start + (OwnerCharacter->GetActorForwardVector() * AttackDistance);

	TArray<FHitResult> HitResults;

	const bool bHit = UKismetSystemLibrary::SphereTraceMultiByProfile(
		GetWorld(),
		Start,
		End,
		AttackRadius,
		"Pawn",
		false,
		{},
		EDrawDebugTrace::None,
		HitResults,
		true
	);

	Multicast_PlayAttackEffects(Start, End, bHit);

	if (!bHit) return;
	
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor != OwnerCharacter && HitActor->GetClass()->ImplementsInterface(UCombatReceiver::StaticClass()))
		{
			if (ICombatReceiver* CombatReceiver = Cast<ICombatReceiver>(HitActor))
			{
				FHitData HitData;
				HitData.Initiator = OwnerCharacter;
				HitData.HitDirection = (Hit.ImpactPoint - OwnerCharacter->GetActorLocation()).GetSafeNormal();
				HitData.HitType = EHitType::Headbutt;

				CombatReceiver->OnHitReceived(HitData);
			}
		}
	}
}

void UHeadbuttComponent::Multicast_PlayAttackEffects_Implementation(const FVector& TraceStart, const FVector& TraceEnd, const bool bHit)
{
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 2.0f, 0, 2.0f);
	if (HeadbuttAnimMontage && OwnerCharacter)
	{
		OwnerCharacter->PlayAnimMontage(HeadbuttAnimMontage);
	}
}