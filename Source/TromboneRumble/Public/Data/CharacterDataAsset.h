// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterDataAsset.generated.h"

UCLASS()
class TROMBONERUMBLE_API UCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config|Movement|Ground")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Movement|Ground")
	float SprintSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Movement|Ground")
	float RotationRate = 360.f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Movement|Air")
	float JumpZVelocity = 500.f;

	UPROPERTY(EditAnywhere, Category = "Config|Movement|Air", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AirControl = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Config|Movement|Air")
	float GravityScale = 1.5f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Movement|Inertia")
	float MaxAcceleration = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Movement|Inertia")
	float BrakingDecelerationWalking = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Movement|Inertia")
	float GroundFriction = 3.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Reactions")
	float RagdollDuration = 2.5f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Reactions")
	float StunDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Config|Camera")
	float TargetArmLength = 650.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Camera")
	float CameraRelativeRotationPitch = -30.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Camera")
	float CameraRelativeLocationZ = 60.0f;
};
