// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmResultWidgetBase.generated.h"

class URhythmSpawnWidgetBase;
/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmResultWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(URhythmSpawnWidgetBase* InOwner);
};
