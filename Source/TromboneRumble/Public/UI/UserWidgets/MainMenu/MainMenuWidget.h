// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "MainMenuWidget.generated.h"

enum class EEasyMatchmakingCompleteResult : uint8;
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
	TObjectPtr<UCommonButtonBase> CB_CreateSession;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_QuickJoin;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Join;
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
	UFUNCTION()
	void HandleCreateSessionClicked();
	UFUNCTION()
	void HandleQuickJoinButtonClicked();
	UFUNCTION()
	void HandleJoinButtonClicked();
	UFUNCTION()
	void HandleCustomizeButtonClicked();
	UFUNCTION()
	void HandleTutorialButtonClicked();
	// ~ End Button Callbacks
	
	/** Displays the tutorial popup */
	void ShowTutorialPopup();
	
	/** Displays the quit confirmation popup */
	void ShowQuitPopup() const;
	
};