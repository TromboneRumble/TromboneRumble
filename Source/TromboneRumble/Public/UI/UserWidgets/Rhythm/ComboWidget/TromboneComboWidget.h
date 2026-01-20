// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RhythmComboWidgetBase.h"
#include "Subsystems/RhythmSubsystem.h"
#include "TromboneComboWidget.generated.h"

class AInstrumentBase;
enum class EInstrumentType : uint8;
enum class ENoteResult : uint8;
enum class EBarInterpType : uint8;
class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API UTromboneComboWidget : public URhythmComboWidgetBase
{
	GENERATED_BODY()

public:
	void SetPercentSmooth(float NewPercent);
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	virtual void HandleComboChanged(ENoteResult InNoteResult, int32 ComboCount);

	UFUNCTION()
	void HandleBuffStatusChanged(bool IsActive);

	void OnBuffActivateAnimationFinished(UUMGSequencePlayer& Player);

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ComboText;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	// Animations
	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> ComboBarAnim;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> ComboTextAnim;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> MissAnim;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> BuffActivateAnim;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> BuffLoopAnim;
	// ~Animations

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EBarInterpType InterpType;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float InterpSpeed = 5.0f;
private:
	void BindDelegates();
	

	float TargetPercent = 0.f;
	float CurrentPercent = 0.0f;
	bool isBuffActivated = false;
};
