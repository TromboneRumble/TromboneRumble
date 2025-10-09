// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TromboneCharacterBase.generated.h"

class UInputComponent;

UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();
	virtual void Tick(float DeltaTime) override;
	

protected:
	virtual void BeginPlay() override;

private:
	void InitCharacter() const;
	void SetupCapsuleComponent() const;
	void SetupSkeletalMeshComponent() const;
	void SetupMovementComponent() const;
	
	void StartRagdoll();
	void StopRagdoll();

private:
	FTimerHandle RagdollTimerHandle;
	float RagdollDuration = 3.0f;
	bool bIsRagdoll = false;
};