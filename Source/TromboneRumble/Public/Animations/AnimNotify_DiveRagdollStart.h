// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_DiveRagdollStart.generated.h"

UCLASS(meta = (DisplayName = "Drunkard Dive Ragdoll Start"))
class TROMBONERUMBLE_API UAnimNotify_DiveRagdollStart : public UAnimNotify
{
	GENERATED_BODY()

public:
	
	//~ Begin UAnimNotify Interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	//~ End UAnimNotify Interface
	
};
