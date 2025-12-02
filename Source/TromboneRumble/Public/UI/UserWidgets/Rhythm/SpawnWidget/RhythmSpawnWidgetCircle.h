// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RhythmSpawnWidgetBase.h"
#include "RhythmSpawnWidgetCircle.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmSpawnWidgetCircle : public URhythmSpawnWidgetBase
{
	GENERATED_BODY()

public:
	virtual void Init(ARhythmNoteSpawner* InNoteSpawner) override;

	virtual URhythmNoteWidgetBase* SpawnPooledRhythmNoteWidget() override;

	virtual URhythmResultWidgetBase* SpawnPooledRhythmResultWidget(const FVector2D& SpawnPos, ENoteResult InResult) override;


protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;


private:
	UFUNCTION()
	void PlayFadeAnimation(EInstrumentType InType);

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* FadeOutAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* FadeInAnim;

};
