// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FloatingRotatingComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UFloatingRotatingComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UFloatingRotatingComponent();

	/** Bob around the location the owner has right now. */
	void ResetBaseLocation();

	/**
	 * Bob around the given location.
	 *
	 * @param InLocation Relative location of the root of the owner.
	 */
	void ResetBaseLocation(const FVector& InLocation);

	/** 위아래 흔들림 속도. 1이면 약 6초, 4면 약 1.5초에 한 번 오르내린다 (rad/s) */
	UPROPERTY(EditAnywhere, Category = "Floating", meta = (DisplayName = "흔들림 속도"))
	float FloatSpeed = 4.0f;

	/** 기준 위치에서 위아래로 움직이는 최대 거리 (cm) */
	UPROPERTY(EditAnywhere, Category = "Floating", meta = (DisplayName = "흔들림 높이"))
	float FloatHeight = 20.0f;

	/** Z축 회전 속도 (도/초) */
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (DisplayName = "회전 속도"))
	float RotationSpeed = 100.0f;

private:

	/** Relative location of the root of the owner the bob is centered on. */
	FVector BaseRelativeLocation = FVector::ZeroVector;

	/** Has the base location been set. The component does nothing until then. */
	bool bIsBaseLocationSet = false;

public:

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface
};
