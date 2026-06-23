// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "InterpolateSpringArmComponent.generated.h"

class ATromboneCharacterBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UInterpolateSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:
	
	UInterpolateSpringArmComponent();
	
	UPROPERTY(EditAnywhere, Category = "InterpolateSpringArm")
	float InterpolationSpeed = 10.0f;

private:
	
	UPROPERTY(Transient)
	TObjectPtr<ATromboneCharacterBase> OwnerCharacter;
	
public:
	
	// ~ Begin USpringArmComponent Interface
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	// ~ End USpringArmComponent Interface
	
};
