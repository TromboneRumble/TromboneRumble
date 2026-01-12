// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmComboWidget.generated.h"

enum class EInstrumentType : uint8;
enum class ENoteResult : uint8;
class UTextBlock;

/**
 * 
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class TROMBONERUMBLE_API URhythmComboWidget : public UUserWidget
{
	GENERATED_BODY()


protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleComboChanged(ENoteResult InNoteResult, int32 ComboCount);

	UFUNCTION()
	void HandleInstrumentChanged(EInstrumentType PrevType, EInstrumentType NewType);

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ComboText;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> BounceAnim;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> MissAnim;

private:
	void BindDelegates();

};
