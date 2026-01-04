// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OptionPanelBase.h"
#include "VideoOptionPanel.generated.h"

UCLASS()
class TROMBONERUMBLE_API UVideoOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void Init(TFunction<void()> BackAction) override;
};
