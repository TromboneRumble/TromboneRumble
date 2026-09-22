// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/FloatableComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/BodySetup.h"
#include "Subsystems/WorldSubsystem/FloatableSubsystem.h"

UFloatableComponent::UFloatableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// Forces added before the step land in this frame's simulation. Later groups add a frame of lag that fights the damping
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UFloatableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UFloatableSubsystem* Subsystem = GetWorld() ? GetWorld()->GetSubsystem<UFloatableSubsystem>() : nullptr)
	{
		Subsystem->Register(this);
	}
}

void UFloatableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFloatableSubsystem* Subsystem = GetWorld() ? GetWorld()->GetSubsystem<UFloatableSubsystem>() : nullptr)
	{
		Subsystem->Unregister(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UFloatableComponent::SetWaterLevel(const float InWaterZ)
{
	WaterZ = InWaterZ;
	SetComponentTickEnabled(true);
}

void UFloatableComponent::EndFloating()
{
	SetWet(false, GetTargetPrimitive());
	SetComponentTickEnabled(false);
}

void UFloatableComponent::SetFloatingAllowed(const bool bInAllowed)
{
	bAllowed = bInAllowed;
	if (!bAllowed)
	{
		SetWet(false, GetTargetPrimitive());
	}
}

void UFloatableComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UPrimitiveComponent* Primitive = GetTargetPrimitive();

	// Any simulating body counts. A held instrument has none, and a ragdoll's root body is kinematic so IsSimulatingPhysics() alone would say no
	if (!bAllowed || !Primitive || !Primitive->IsAnySimulatingPhysics())
	{
		SetWet(false, Primitive);
		return;
	}

	if (!bWet && Primitive->Bounds.GetBox().Min.Z < WaterZ)
	{
		SetWet(true, Primitive);
	}
	if (!bWet) return;

	// Corner sampling needs one body. Skeletal meshes have many small bodies that already tilt on their own
	if (bSampleBoundsCorners && !Primitive->IsA<USkeletalMeshComponent>())
	{
		if (FBodyInstance* Body = Primitive->GetBodyInstance())
		{
			ApplyBuoyancyAtCorners(*Primitive, Body);
		}
		return;
	}

	ForEachBody(*Primitive, [this](FBodyInstance* Body) { ApplyBuoyancyToBody(Body); });
}

UPrimitiveComponent* UFloatableComponent::GetTargetPrimitive() const
{
	if (UPrimitiveComponent* Override = TargetPrimitive.Get())
	{
		return Override;
	}
	const AActor* Owner = GetOwner();
	return Owner ? Cast<UPrimitiveComponent>(Owner->GetRootComponent()) : nullptr;
}

void UFloatableComponent::ForEachBody(UPrimitiveComponent& Primitive, const TFunctionRef<void(FBodyInstance*)> Fn) const
{
	if (USkeletalMeshComponent* Skeletal = Cast<USkeletalMeshComponent>(&Primitive))
	{
		Skeletal->ForEachBodyBelow(NAME_None, true, false, Fn);
		return;
	}
	if (FBodyInstance* Body = Primitive.GetBodyInstance())
	{
		Fn(Body);
	}
}

float UFloatableComponent::GetBuoyancyAccel() const
{
	const float Gravity = GetWorld() ? FMath::Abs(GetWorld()->GetGravityZ()) : 980.f;
	return Gravity / FMath::Max(0.05f, RelativeDensity);
}

float UFloatableComponent::GetFullDepth(const FVector& Size) const
{
	return FullSubmersionDepth > 0.f ? FullSubmersionDepth : FMath::Max(1.f, Size.GetAbsMin() * 0.5f);
}

float UFloatableComponent::GetSubmersion(const float Z, const float FullDepth) const
{
	return FMath::Clamp((WaterZ - Z) / FullDepth, 0.f, 1.f);
}

void UFloatableComponent::ApplyBuoyancyToBody(FBodyInstance* Body) const
{
	const FBox Bounds = Body->GetBodyBounds();
	const float Submersion = GetSubmersion(Bounds.GetCenter().Z, GetFullDepth(Bounds.GetSize()));
	if (Submersion <= 0.f) return;

	Body->AddForce(FVector::UpVector * GetBuoyancyAccel() * Submersion, true, true);
}

void UFloatableComponent::ApplyBuoyancyAtCorners(const UPrimitiveComponent& Primitive, FBodyInstance* Body) const
{
	// Collision box matches the mass better than the render bounds
	const UBodySetup* Setup = Body->GetBodySetup();
	FBox LocalBox = Setup ? Setup->AggGeom.CalcAABB(FTransform::Identity) : FBox(ForceInit);
	if (!LocalBox.IsValid)
	{
		LocalBox = Primitive.GetLocalBounds().GetBox();
	}

	const FTransform& ToWorld = Primitive.GetComponentTransform();
	const float FullDepth = GetFullDepth(LocalBox.GetSize() * ToWorld.GetScale3D());
	// AddForceAtPosition takes a force, not an acceleration, so scale by mass here
	const float MassPerCorner = Body->GetBodyMass() / 8.f;
	const float Accel = GetBuoyancyAccel();

	for (int32 Index = 0; Index < 8; ++Index)
	{
		const FVector LocalCorner(
			(Index & 1) ? LocalBox.Max.X : LocalBox.Min.X,
			(Index & 2) ? LocalBox.Max.Y : LocalBox.Min.Y,
			(Index & 4) ? LocalBox.Max.Z : LocalBox.Min.Z);
		const FVector Corner = ToWorld.TransformPosition(LocalCorner);

		const float Submersion = GetSubmersion(Corner.Z, FullDepth);
		if (Submersion <= 0.f) continue;

		const float VerticalSpeed = Body->GetUnrealWorldVelocityAtPoint(Corner).Z;
		const float Force = MassPerCorner * Submersion * (Accel - VerticalSpeed * CornerDrag);
		Body->AddForceAtPosition(FVector::UpVector * Force, Corner, true);
	}
}

void UFloatableComponent::ApplyWetTilt(UPrimitiveComponent& Primitive) const
{
	if (WetTiltSpeed <= 0.f || Primitive.IsA<USkeletalMeshComponent>()) return;

	FBodyInstance* Body = Primitive.GetBodyInstance();
	if (!Body) return;

	const AActor* Owner = GetOwner();
	FRandomStream Stream(Owner ? static_cast<int32>(GetTypeHash(Owner->GetFName())) : 0);
	FVector Axis = Stream.VRand();
	Axis.Z = 0.f;
	if (!Axis.Normalize())
	{
		Axis = FVector::ForwardVector;
	}

	Body->SetAngularVelocityInRadians(Axis * FMath::DegreesToRadians(WetTiltSpeed), true);
}

void UFloatableComponent::SetWet(const bool bNewWet, UPrimitiveComponent* Primitive)
{
	if (bWet == bNewWet) return;
	bWet = bNewWet;

	if (!Primitive) return;

	if (bWet)
	{
		if (const FBodyInstance* First = Primitive->GetBodyInstance())
		{
			SavedLinearDamping = First->LinearDamping;
			SavedAngularDamping = First->AngularDamping;
		}
		// A prop resting on the floor is asleep and would ignore the force
		Primitive->WakeAllRigidBodies();
		ApplyWetTilt(*Primitive);
	}

	const float Linear = bWet ? WaterLinearDamping : SavedLinearDamping;
	const float Angular = bWet ? WaterAngularDamping : SavedAngularDamping;
	ForEachBody(*Primitive, [Linear, Angular](FBodyInstance* Body)
	{
		Body->LinearDamping = Linear;
		Body->AngularDamping = Angular;
		Body->UpdateDampingProperties();
	});
}
