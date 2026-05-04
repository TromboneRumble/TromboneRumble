// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OptionPanelBase.h"
#include "GameplayOptionPanel.generated.h"

class UOptionCycleRowWidget;
class UCheckBoxRowWidget;

UCLASS()
class TROMBONERUMBLE_API UGameplayOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()
	
public:
	
	// ~ Begin UOptionPanelBase Interface
	virtual void RefreshUI() override;
	virtual void ApplySettingsFromUI(bool bSaveToDisk) override;
	virtual void ApplySettingsFromSavedData() override;
	virtual bool IsDirty() const override;
	// ~ End UOptionPanelBase Interface

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBoxRowWidget> CBR_ShouldShowUsernameInGame;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOptionCycleRowWidget> OC_VOIP;
};
