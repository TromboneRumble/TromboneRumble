// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "InGameWidget.generated.h"

class UCanvasPanel;
enum class EInGameState : uint8;

UCLASS()
class TROMBONERUMBLE_API UInGameWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown, EMouseLockMode::LockAlways, true);
	}	
	
	void ToggleGuideUI();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> InGameCanvas;

protected:
	virtual void NativeConstruct() override;

	void BindToInGameState(AGameStateBase* NewGameState);

	UFUNCTION()
	void HandleInGameStateChanged(EInGameState NewState);
};
