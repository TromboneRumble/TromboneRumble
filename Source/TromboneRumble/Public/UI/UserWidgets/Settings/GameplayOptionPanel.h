// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OptionPanelBase.h"
#include "GameplayOptionPanel.generated.h"

class UOptionCycleWidget;
class UCommonCheckBoxRowWidget;

UCLASS()
class TROMBONERUMBLE_API UGameplayOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()
	
protected:
	
	// ~ Begin UOptionPanelBase Interface
	virtual void RefreshUI() override;
	virtual void ReapplySavedSettings() override;
	virtual void HandleApplyButtonClicked() override;
	virtual void HandleResetButtonClicked() override;
	// ~ End UOptionPanelBase Interface
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonCheckBoxRowWidget> CBR_ShouldShowUsernameInGame;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOptionCycleWidget> OC_VOIP;
};
