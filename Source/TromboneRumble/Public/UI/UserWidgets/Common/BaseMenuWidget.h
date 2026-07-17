// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "BaseMenuWidget.generated.h"

UCLASS(Abstract)
class TROMBONERUMBLE_API UBaseMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	/** Default constructor. */
	UBaseMenuWidget();

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, EMouseLockMode::LockOnCapture, false);
	}	

	/** Initialize menu widget */
	virtual void Init() PURE_VIRTUAL(UBaseMenuWidget::Init, );
	
	/** Enable/disable the interactable UI elements within the widget */
	virtual void SetUIEnabled(const bool bEnabled) PURE_VIRTUAL(UBaseMenuWidget::SetUIEnabled, );
	
protected:
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual void NativeConstruct() override;
	// ~ End UCommonActivatableWidget Interface
	
};
