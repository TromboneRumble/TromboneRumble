// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Rhythm/ComboWidget/RhythmComboWidgetBase.h"
#include "ViolinComboWidget.generated.h"

class UCanvasPanel;
class AInstrumentBase;
enum class EInstrumentType : uint8;
enum class ENoteResult : uint8;
enum class EBarInterpType : uint8;
class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UViolinComboWidget : public URhythmComboWidgetBase
{
	GENERATED_BODY()
public:
	void SetPercentSmooth(float NewPercent);
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
	void HandleComboChanged(ENoteResult InNoteResult, int32 ComboCount);

	UFUNCTION(BlueprintNativeEvent)
	void HandleBuffStatusChanged(bool IsActive);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	// Animations
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> ComboBarAnim;

	// ~Animations

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EBarInterpType InterpType;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float InterpSpeed = 5.0f;
private:
	void BindDelegates();


	float TargetPercent = 0.f;
	float CurrentPercent = 0.0f;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool isBuffActivated = false;
};
