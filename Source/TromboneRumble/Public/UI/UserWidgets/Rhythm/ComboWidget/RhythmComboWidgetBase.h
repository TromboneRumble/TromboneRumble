// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/InstrumentBase.h"
#include "RhythmComboWidgetBase.generated.h"

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
	TWeakObjectPtr<AInstrumentBase> OwnerInstrument;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UTexture> PerfectComboImg;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UTexture> GoodComboImg;
};
