// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UPrimitiveComponent;
class USkeletalMeshComponent;

/**
 * Body settings shared by every physics object characters can push, such as props and dropped instruments.
 * They cap the speed a body gets when it is pushed out of an overlap, and its spin.
 * Without the caps, a light body under a character can pop up and fling the character away.
 */
namespace PushablePhysics
{
	/** Speed that pushes a body out when it starts inside another body (cm/s). */
	inline constexpr float MaxDepenetrationVelocity = 150.f;

	/** Spin limit of a body (degrees/s). */
	inline constexpr float MaxAngularVelocity = 720.f;

	/**
	 * Put the caps on the body template of the component, and keep characters from stepping up or standing on it.
	 * Every face becomes too steep to land on, so a character that jumps onto it slides off.
	 * Call it in a constructor, so a Blueprint can still change the values.
	 */
	TROMBONERUMBLE_API void ApplyDefaults(UPrimitiveComponent& Primitive);

	/**
	 * Copy the caps of the component template onto every body of the skeletal mesh.
	 * Keep in mind that skeletal bodies come from the physics asset and ignore the component template.
	 * Call it each time the physics state is created.
	 */
	TROMBONERUMBLE_API void CopyCapsToBodies(USkeletalMeshComponent& Skeletal);
}
