// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmComboWidgetBase.generated.h"

class AInstrumentBase;

/**
 * 
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class TROMBONERUMBLE_API URhythmComboWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void Init(AInstrumentBase* InOwner);

	UPROPERTY(Transient)
	TObjectPtr<AInstrumentBase> OwnerInstrument;
};
