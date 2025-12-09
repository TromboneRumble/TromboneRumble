// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RhythmNoteWidgetBase.h"
#include "RhythmNoteWidgetCircle.generated.h"

class USizeBox;
/**
 * 
 */
UCLASS(Abstract, meta = (DiableNativeTick))
class TROMBONERUMBLE_API URhythmNoteWidgetCircle : public URhythmNoteWidgetBase
{
	GENERATED_BODY()
public:
	virtual void Init(const EInstrumentType& InType) override;
	virtual void UpdateNotePosition(const float InAlpha) override;

private:
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	float StartRadius = 1050.f;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	float FinishRadius = 550.f;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* FadeOutAnimation;
};
