// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OptionPanelBase.h"
#include "AudioOptionPanel.generated.h"

UCLASS()
class TROMBONERUMBLE_API UAudioOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void Init(TFunction<void()> BackAction) override;
	
};
