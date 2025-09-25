// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TromboneCharacterBase.generated.h"

UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

private:

};
