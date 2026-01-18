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
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	
	virtual void Init();
	virtual void ShowNoticePopup(const FString& Content);
	virtual void BindSubsystemCallbacks();
	virtual void RemoveSubsystemCallbacks();
	virtual void SetUIEnabled(const bool bEnabled);
	
	void ChangeMenu(EMainMenuType InType) const;
	TObjectPtr<UMainUIRoot> GetRootLayout() const;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NoticePopupWidgetClass;
	
	UPROPERTY(Transient)
	TObjectPtr<UEasySessionSubsystem> SessionsSubsystem;
};
