// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UfoPath.generated.h"

/**
 * Straight path of a UFO, described by its two ends and the server time it started.
 * Every machine computes the same position from it, so nothing else about the movement is replicated.
 */
USTRUCT(BlueprintType)
struct FUfoPath
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Start = FVector::ZeroVector;

	/** Same as Start when the UFO does not move. */
	UPROPERTY(BlueprintReadOnly)
	FVector End = FVector::ZeroVector;

	/** Server world time the UFO left Start. */
	UPROPERTY(BlueprintReadOnly)
	float StartServerTime = 0.f;

	/** Seconds from Start to End. 0 when the UFO does not move. */
	UPROPERTY(BlueprintReadOnly)
	float Duration = 0.f;

	/** @return Position on the path at this server time. Stays at End after the path is done. */
	FVector Evaluate(const float ServerTime) const
	{
		if (Duration <= 0.f) return Start;

		const float Alpha = FMath::Clamp((ServerTime - StartServerTime) / Duration, 0.f, 1.f);
		return FMath::Lerp(Start, End, Alpha);
	}
};
