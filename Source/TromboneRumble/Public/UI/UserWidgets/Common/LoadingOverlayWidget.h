// Copyright (C) 2026 biksari studio. All Rights Reserved.

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
	
	UFUNCTION(BlueprintCallable, Category = "LoadingWidget")
	virtual void InitWithContent(const FText& InContent = FText::GetEmpty());
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	FText DefaultContent = FText::FromString("Loading...");
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Content;
};