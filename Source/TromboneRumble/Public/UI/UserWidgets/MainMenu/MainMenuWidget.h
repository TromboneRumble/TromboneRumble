// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "MainMenuWidget.generated.h"

class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UMainMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()
	
public:
	
	// ~ Begin UBaseMenuWidget Interface
	virtual void Init() override;
	virtual void SetUIEnabled(const bool bEnabled) override;
	// ~ End UBaseMenuWidget Interface
	
protected:
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual void NativeOnInitialized() override;
	// ~ End UCommonActivatableWidget Interface

protected:
	
	// ~ Begin UI
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Play;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Customize;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Settings;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Tutorial;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Quit;
	// ~ End UI
	
private:
	
	// ~ Begin Button Callbacks
	void HandlePlayButtonClicked();
	void HandleCustomizeButtonClicked();
	void HandleTutorialButtonClicked();
	// ~ End Button Callbacks
	
	bool TryShowFirstTutorialPopup() const;

	/** Displays the tutorial popup */
	void ShowTutorialPopup() const;

	/** Displays the quit confirmation popup */
	void ShowQuitPopup() const;
	
};