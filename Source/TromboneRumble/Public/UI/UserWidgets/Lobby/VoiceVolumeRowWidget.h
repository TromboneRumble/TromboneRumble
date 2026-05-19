// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "VoiceVolumeRowWidget.generated.h"

class ADefaultPlayerState;
class UAnalogSlider;
class UTextBlock;
class UProgressBar;
class UVoiceChatSubsystem;
class USaveManagerSubsystem;

UCLASS()
class TROMBONERUMBLE_API UVoiceVolumeRowWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAnalogSlider> Slider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Value;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar;

	void Init(ADefaultPlayerState* InPS, bool bIsLocal);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	TWeakObjectPtr<ADefaultPlayerState> WeakPS;
	bool bIsLocalPlayerRow = false;

	UFUNCTION()
	void HandleSliderValueChanged(float Value);

	UVoiceChatSubsystem* GetVCS() const;
	USaveManagerSubsystem* GetSaveManager() const;
};
