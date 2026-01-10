// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FloatingRotatingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TROMBONERUMBLE_API UFloatingRotatingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFloatingRotatingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ResetBaseLocation();
	void ResetBaseLocation(const FVector& InLocation);
public:
	UPROPERTY(EditAnywhere, Category = "Floating")
	float FloatSpeed = 4.0f; // 위아래 왕복 속도

	UPROPERTY(EditAnywhere, Category = "Floating")
	float FloatHeight = 20.0f; // 위아래 이동 범위 (Amplitude)

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float RotationSpeed = 100.0f;

protected:
	virtual void BeginPlay() override;

private:
	FVector BaseRelativeLocation;
	bool bIsBaseLocationSet = false;

};
