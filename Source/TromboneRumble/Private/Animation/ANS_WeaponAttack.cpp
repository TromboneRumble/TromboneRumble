// Copyright (C) 2026 biksari studio. All Rights Reserved.

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
			Weapon->BeginAttack(TotalDuration);
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
