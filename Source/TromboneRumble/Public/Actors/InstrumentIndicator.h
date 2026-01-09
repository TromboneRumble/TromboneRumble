// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InstrumentIndicator.generated.h"

class UFloatingRotatingComponent;

UCLASS(Abstract)
class TROMBONERUMBLE_API AInstrumentIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	AInstrumentIndicator();
	void ResetBaseLocation(const FVector& InLocation);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UFloatingRotatingComponent> FloatingRotatingComponent;
};
