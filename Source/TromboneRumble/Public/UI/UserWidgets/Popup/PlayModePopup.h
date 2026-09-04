// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PopupBase.h"
#include "PlayModePopup.generated.h"

class UCommonTextBlock;
class UEditableText;
enum class ECommonInputType : uint8;

UCLASS()
class TROMBONERUMBLE_API UPlayModePopup : public UPopupBase
{
	GENERATED_BODY()

protected:

	//~ Begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	//~ End UUserWidget Interface

	//~ Begin UPopupBase Interface
	virtual void Register() override;
	virtual void Unregister() override;
	virtual UWidget* GetDefaultFocusWidget() const override;
	//~ End UPopupBase Interface

protected:

	// ~ Begin UI
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_CreateSession;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_QuickJoin;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_JoinCode;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_Code;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Join;

	/** Shown only while a gamepad is in use. Tells the player to type the code with the keyboard. */
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_GamepadHint;
	// ~ End UI

private:

	// ~ Begin Button Callbacks
	void HandleCreateSessionClicked();
	void HandleQuickJoinClicked();
	void HandleJoinCodeClicked();
	void HandleJoinClicked();
	// ~ End Button Callbacks

	/** Shows the keyboard hint while a gamepad is in use. Windows has no on-screen keyboard. */
	void HandleInputMethodChanged(ECommonInputType NewInputType);
};
