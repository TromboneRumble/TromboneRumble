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
	
	UFUNCTION(BlueprintCallable, Category = "Loading")
	virtual void InitDefault();
	
	UFUNCTION(BlueprintCallable, Category = "Loading")
	virtual void InitWithContent(const FString& InContent = TEXT(""));
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	FString DefaultContent = TEXT("Loading...");
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Content;
};