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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	virtual void HandleComboChanged(ENoteResult InNoteResult, int32 ComboCount);

	UFUNCTION()
	void HandleOnAttack(AActor* HitActor);

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ComboText;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UImage> CymbalsImg;

	// Animations
	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> OnAttackAnim;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> MissAnim;
	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> ComboTextAnim;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> IdleAnim;
	// ~Animations
private:
	void BindDelegates();
};
