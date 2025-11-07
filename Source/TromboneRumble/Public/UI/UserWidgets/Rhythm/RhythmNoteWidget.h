// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmNoteWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmNoteWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

};
