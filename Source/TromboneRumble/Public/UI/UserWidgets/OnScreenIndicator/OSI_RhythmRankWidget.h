// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSI_WidgetBase.h"
#include "Utilities/Defines.h"
#include "OSI_RhythmRankWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class TROMBONERUMBLE_API UOSI_RhythmRankWidget : public UOSI_WidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleRhythmGameStateChanged(ERhythmGameState RhythmGameState);
};
