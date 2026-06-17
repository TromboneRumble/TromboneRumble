// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "VoiceVolumeRowWidget.generated.h"

class UButton;
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
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Plus;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Minus;

	void Init(ADefaultPlayerState* InPS, bool bIsLocal);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	TWeakObjectPtr<ADefaultPlayerState> WeakPS;
	bool bIsLocalPlayerRow = false;

	UFUNCTION()
	void HandleSliderValueChanged(float Value);

	// 버튼 핸들러 (UButton OnPressed/OnReleased는 dynamic delegate → UFUNCTION 필수)
	UFUNCTION() void HandlePlusPressed();
	UFUNCTION() void HandleMinusPressed();
	UFUNCTION() void HandleButtonReleased();

	// 타이머 콜백 (FTimerDelegate, UFUNCTION 불필요)
	void BeginContinuousAdjust();
	void HandleHoldRepeat();

	// 공통 헬퍼
	void StartHold(int32 Direction);
	void StopHold();
	void ApplyDelta(float DeltaSliderValue);

	// 상태
	FTimerHandle HoldDelayTimerHandle;   // 꾹 누르기 → 연속 조절 시작까지 지연 (one-shot)
	FTimerHandle HoldRepeatTimerHandle;  // 연속 조절 (looping)
	int32 HoldDirection = 0;             // +1 / -1
	float ContinuousElapsed = 0.0f;      // 연속 조절 시작 후 경과 시간(초)

	// 디자이너 튜닝용 파라미터 (% / 초 단위)
	UPROPERTY(EditDefaultsOnly, Category = "Voice Volume Buttons")
	float ClickStepPercent = 1.0f;        // 단일 클릭 스텝 (%)
	UPROPERTY(EditDefaultsOnly, Category = "Voice Volume Buttons")
	float HoldStartDelay = 0.8f;          // 꾹 누르기 → 연속 시작까지 지연 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Voice Volume Buttons")
	float RepeatInterval = 0.03f;         // 연속 조절 틱 간격 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Voice Volume Buttons")
	float MinHoldSpeedPercent = 8.0f;     // 연속 시작 속도 (%/초)
	UPROPERTY(EditDefaultsOnly, Category = "Voice Volume Buttons")
	float MaxHoldSpeedPercent = 200.0f;   // 연속 최대 속도 (%/초)
	UPROPERTY(EditDefaultsOnly, Category = "Voice Volume Buttons")
	float HoldAccelGrowth = 3.6f;         // 초당 속도 배율 (지수 가속)

	UVoiceChatSubsystem* GetVCS() const;
	USaveManagerSubsystem* GetSaveManager() const;
};
