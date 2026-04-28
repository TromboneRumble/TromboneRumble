// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Data/UIData.h"
#include "ToastContainerWidget.generated.h"

class UOverlay;

UCLASS()
class TROMBONERUMBLE_API UToastContainerWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	
	/** Add a toast item widget to the container. */
	void AddToastItem(UUserWidget* InToastItem, const EToastPosition InPosition);
	
	/** @return Current active toast widget */
	UUserWidget* GetCurrentActiveToast();
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> ToastAnchorOverlay;
	
private:
	
	/** Reference to the currently active toast widget */
	TWeakObjectPtr<UUserWidget> CurrentActiveToast;
	
};
