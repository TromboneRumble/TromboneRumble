// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
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
class TROMBONERUMBLE_API UMainUIRoot : public UBaseUIRoot
{
	GENERATED_BODY()
	
public:
	void PushMenu(EMainMenuType InType) const;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> SettingMenuWidgetClass;
};