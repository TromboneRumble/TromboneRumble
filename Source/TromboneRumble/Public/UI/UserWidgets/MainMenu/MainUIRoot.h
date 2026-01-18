// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MainUIRoot.generated.h"

class UCommonActivatableWidgetStack;
class UCommonActivatableWidget;

enum class EMainMenuType : uint8
{
	None,
	MainMenu,
	Settings
};

UCLASS()
class TROMBONERUMBLE_API UMainUIRoot : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;
	void PushMenu(EMainMenuType InType) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuStack;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> MainMenuWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> SettingMenuWidgetClass;
};