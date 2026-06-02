// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RhythmComboWidgetBase.h"
#include "Subsystems/RhythmSubsystem.h"
#include "CymbalsComboWidget.generated.h"


class AInstrumentBase;
enum class EInstrumentType : uint8;
enum class ENoteResult : uint8;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API UCymbalsComboWidget : public URhythmComboWidgetBase
{
	GENERATED_BODY()

public:
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintNativeEvent)
	void HandleComboChanged(ENoteResult InNoteResult, int32 ComboCount);

	UFUNCTION(BlueprintImplementableEvent)
	void HandleOnAttack(AActor* HitActor);

	FTimerHandle BindRetryTimerHandle;

	// Animations

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> MissAnim;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> ComboTextAnim;

	// ~Animations
private:
	void BindDelegates();
};
