// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Utilities/PushablePhysics.h"
#include "PushablePropActor.generated.h"

/**
 * APushablePropActor is a physics prop that characters can push, such as the chairs of the jazz bar.
 * It simulates physics, replicates its movement, and caps the push-out speed and the spin of its body.
 * Place it, or a Blueprint of it, instead of a plain static mesh actor, so these settings live in one class.
 * Keep in mind that the Pushable values own the matching settings of the mesh component and overwrite them.
 *
 * @see PushablePhysics
 */
UCLASS()
class TROMBONERUMBLE_API APushablePropActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	APushablePropActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin AActor Interface
	virtual void OnConstruction(const FTransform& Transform) override;
	//~ End AActor Interface

protected:

	/** 캐릭터와 겹쳤다가 밀려날 때의 속도 상한. 낮을수록 밟혀도 덜 튄다 */
	UPROPERTY(EditAnywhere, Category = "Pushable", meta = (DisplayName = "튀는 속도 상한", ClampMin = "0.0", Units = "cm/s"))
	float MaxDepenetrationVelocity = PushablePhysics::MaxDepenetrationVelocity;

	/** 회전 속도 상한. 낮을수록 팽이처럼 돌지 않는다 */
	UPROPERTY(EditAnywhere, Category = "Pushable", meta = (DisplayName = "회전 속도 상한", ClampMin = "0.0", Units = "deg/s"))
	float MaxAngularVelocity = PushablePhysics::MaxAngularVelocity;

	/** 걷다가 턱처럼 올라설 수 있는지. 점프해서 올라서는 것은 막지 못한다 */
	UPROPERTY(EditAnywhere, Category = "Pushable", meta = (DisplayName = "걸어서 올라서기"))
	TEnumAsByte<ECanBeCharacterBase> CanCharacterStepUpOn = ECB_No;

	/** 켜면 캐릭터가 이 소품 위에 서지 못하고 미끄러진다. 윗면이 평평하면 캐릭터가 위에서 낙하 상태로 멈출 수 있다 */
	UPROPERTY(EditAnywhere, Category = "Pushable", meta = (DisplayName = "위에 서지 못함"))
	bool bUnwalkableTop = true;
};
