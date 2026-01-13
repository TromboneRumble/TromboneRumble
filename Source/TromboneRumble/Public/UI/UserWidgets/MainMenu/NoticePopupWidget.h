// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/MainMenu/PopupWidgetBase.h"
#include "NoticePopupWidget.generated.h"

class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UNoticePopupWidget : public UPopupWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void OnInit(FString InContent);
	
protected:
	// ~ Begin Widgets
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Content;
	// ~ End Widgets
};
