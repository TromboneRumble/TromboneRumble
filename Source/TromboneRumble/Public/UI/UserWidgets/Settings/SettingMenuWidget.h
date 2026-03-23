// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "SettingMenuWidget.generated.h"

class UCommonAnimatedSwitcher;
class UCommonButtonBase;
class UVideoOptionPanel;
class UAudioOptionPanel;

UCLASS()
class TROMBONERUMBLE_API USettingMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()
	
protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	
	virtual void Init() override;
	
private:
	void ChangePanel(UWidget* TargetWidget);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonAnimatedSwitcher> CAS_Settings;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAudioOptionPanel> Widget_AudioOptions;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVideoOptionPanel> Widget_VideoOptions;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Audio;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Video;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Language;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Back;
};
