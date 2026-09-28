// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Props/PushablePropActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Utilities/PushablePhysics.h"

APushablePropActor::APushablePropActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// AStaticMeshActor turns replication on in BeginPlay from this flag
	bStaticMeshReplicateMovement = true;
	bReplicates = true;
	SetReplicatingMovement(true);

	if (UStaticMeshComponent* Mesh = GetStaticMeshComponent())
	{
		Mesh->Mobility = EComponentMobility::Movable;
		Mesh->bUseDefaultCollision = false;
		Mesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
		Mesh->SetSimulatePhysics(true);
		PushablePhysics::ApplyDefaults(*Mesh);
	}
}

void APushablePropActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UStaticMeshComponent* Mesh = GetStaticMeshComponent();
	if (!Mesh) return;

	Mesh->CanCharacterStepUpOn = CanCharacterStepUpOn;
	Mesh->BodyInstance.SetMaxDepenetrationVelocity(MaxDepenetrationVelocity);
	Mesh->BodyInstance.SetMaxAngularVelocityInRadians(FMath::DegreesToRadians(MaxAngularVelocity), false);

	// The toggle owns this setting, so turning it off puts the default slope rule back
	// Unwalkable makes every face too steep to land on, so a character slides off
	Mesh->SetWalkableSlopeOverride(bUnwalkableTop ? FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f) : FWalkableSlopeOverride());
}
