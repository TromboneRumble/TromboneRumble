// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "BaseMenuWidget.generated.h"

enum class EMainMenuType : uint8;
class UMainUIRoot;
class UEasySessionSubsystem;

UCLASS()
class TROMBONERUMBLE_API UBaseMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::CaptureDuringMouseDown);
	}	

protected:
	virtual void NativeConstruct() override;
	
	virtual void Init();
	virtual void ShowNoticePopup(const FString& Content);
	virtual void BindSubsystemCallbacks();
	virtual void RemoveSubsystemCallbacks();
	virtual void SetUIEnabled(const bool bEnabled);
	
	virtual void ShowLoadingOverlay();
	virtual void ShowLoadingOverlay(FString InContent);
	virtual void HideLoadingOverlay();
	
	void SwitchMenu(EMainMenuType InType);
	TObjectPtr<UMainUIRoot> GetRootLayout() const;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NoticePopupWidgetClass;
	
	UPROPERTY(Transient)
	TObjectPtr<UEasySessionSubsystem> SessionsSubsystem;
};
