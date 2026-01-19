// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "LoadingOverlayWidget.generated.h"

class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API ULoadingOverlayWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::CaptureDuringMouseDown);
	}
	
	virtual void Init();
	virtual void Init(FString InContent);
	
protected:
	UPROPERTY(EditDefaultsOnly)
	FString DefaultContent = TEXT("Loading...");
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Content;
};