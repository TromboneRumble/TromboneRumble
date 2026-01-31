// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TutorialDummy.generated.h"

UCLASS()
class TROMBONERUMBLE_API ATutorialDummy : public APawn
{
	GENERATED_BODY()

public:
	ATutorialDummy();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

};
