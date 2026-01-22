// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "LobbyWidget.generated.h"

class UCommonTextBlock;
enum class ELobbyState : uint8;

UCLASS()
class TROMBONERUMBLE_API ULobbyWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown, EMouseLockMode::LockAlways, true);
	}	
	
protected:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnLobbyStateUpdated(ELobbyState NewState);

	void UpdateCountdown();
	
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	int32 CountdownSeconds = 5;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Countdown;

	FTimerHandle CountdownTimerHandle;
};