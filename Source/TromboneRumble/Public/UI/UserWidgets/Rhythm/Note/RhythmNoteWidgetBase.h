// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmNoteWidgetBase.generated.h"

class URhythmSpawnWidgetBase;
/**
 * 
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class TROMBONERUMBLE_API URhythmNoteWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void Init(URhythmSpawnWidgetBase* InOwner);
	virtual void InitWithCueMessage(const FString& InUserCueName){};

	virtual void UpdateNotePosition(const float InAlpha){};

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<URhythmSpawnWidgetBase> OwnerSpawnWidget;
	
};
