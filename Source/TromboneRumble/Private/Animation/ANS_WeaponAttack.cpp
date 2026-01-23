// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/ANS_WeaponAttack.h"

#include "Characters/DefaultTromboneCharacter.h"
#include "Items/WeaponBase.h"

void UANS_WeaponAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                    const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (const ADefaultTromboneCharacter* Character = Cast<ADefaultTromboneCharacter>(MeshComp->GetOwner()))
	{
		if (!Character->HasAuthority()) return;
		
		if (AWeaponBase* Weapon = Character->GetCurrentWeapon())
		{
			Weapon->BeginAttack();
		}
	}
}

void UANS_WeaponAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (const ADefaultTromboneCharacter* Character = Cast<ADefaultTromboneCharacter>(MeshComp->GetOwner()))
	{
		if (!Character->HasAuthority()) return;
		
		if (AWeaponBase* Weapon = Character->GetCurrentWeapon())
		{
			Weapon->EndAttack();
		}
	}
}
