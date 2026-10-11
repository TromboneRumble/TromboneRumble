// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Utilities/PushablePhysics.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"

void PushablePhysics::ApplyDefaults(UPrimitiveComponent& Primitive)
{
	Primitive.CanCharacterStepUpOn = ECB_No;
	Primitive.SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));

	FBodyInstance& Body = Primitive.BodyInstance;
	Body.SetMaxDepenetrationVelocity(MaxDepenetrationVelocity);
	Body.bOverrideMaxAngularVelocity = true;
	Body.MaxAngularVelocity = MaxAngularVelocity;
}

void PushablePhysics::CopyCapsToBodies(USkeletalMeshComponent& Skeletal)
{
	const FBodyInstance& Template = Skeletal.BodyInstance;
	Skeletal.ForEachBodyBelow(NAME_None, true, false, [&Template](FBodyInstance* Body)
	{
		if (Template.GetOverrideMaxDepenetrationVelocity())
		{
			Body->SetMaxDepenetrationVelocity(Template.GetMaxDepenetrationVelocity());
		}
		if (Template.bOverrideMaxAngularVelocity)
		{
			Body->SetMaxAngularVelocityInRadians(FMath::DegreesToRadians(Template.MaxAngularVelocity), false);
		}
	});
}
