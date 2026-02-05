// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PopupWidgetBase.h"
#include "NoticePopupWidget.generated.h"

class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UNoticePopupWidget : public UPopupWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void OnInit(FString InContent);
	virtual void OnInit(FString InTitle, FString InContent);
	
protected:
	// ~ Begin Widgets
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Title;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Content;
	// ~ End Widgets
};
