// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmUIRootWidget.generated.h"


class UCanvasPanel;
class URhythmSpawnWidget;

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmUIRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NoteCanvas = nullptr;


};
