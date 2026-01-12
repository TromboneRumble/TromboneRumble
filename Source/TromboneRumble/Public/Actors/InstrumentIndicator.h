// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InstrumentIndicator.generated.h"

class AInstrumentBase;
class UFloatingRotatingComponent;

UCLASS(Abstract)
class TROMBONERUMBLE_API AInstrumentIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	AInstrumentIndicator();
	void InitInstrument(AInstrumentBase* InInstrumentBase, const FVector& InIndicatorOffset);
	void ResetBaseLocation(const FVector& InLocation);

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UFloatingRotatingComponent> FloatingRotatingComponent;

	UPROPERTY()
	FVector IndicatorOffset = FVector(0.0f, 0.0f, 100.0f);

	UPROPERTY(Transient)
	TWeakObjectPtr<AInstrumentBase> OwnerInstrument = nullptr;
};
