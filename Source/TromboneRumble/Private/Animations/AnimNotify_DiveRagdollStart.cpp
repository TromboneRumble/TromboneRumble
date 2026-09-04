// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Animations/AnimNotify_DiveRagdollStart.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"

void UAnimNotify_DiveRagdollStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (ADrunkardNPC* Drunkard = MeshComp ? Cast<ADrunkardNPC>(MeshComp->GetOwner()) : nullptr)
	{
		Drunkard->HandleDiveRagdollStart();
	}
}
