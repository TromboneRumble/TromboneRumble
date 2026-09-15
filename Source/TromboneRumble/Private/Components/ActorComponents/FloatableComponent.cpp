// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/FloatableComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Subsystems/WorldSubsystem/FloatableSubsystem.h"

UFloatableComponent::UFloatableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
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

void UFloatableComponent::ApplyBuoyancyToBody(FBodyInstance* Body) const
{
	const float Depth = WaterZ - Body->GetUnrealWorldTransform().GetLocation().Z;
	if (Depth <= 0.f) return;

	const float Submersion = FMath::Clamp(Depth / FMath::Max(1.f, FullSubmersionDepth), 0.f, 1.f);
	Body->AddForce(FVector::UpVector * BuoyancyAccel * Submersion, true, true);
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
