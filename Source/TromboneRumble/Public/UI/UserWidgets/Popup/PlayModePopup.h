// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PopupBase.h"
#include "PlayModePopup.generated.h"

class UEditableText;

UCLASS()
class TROMBONERUMBLE_API UPlayModePopup : public UPopupBase
{
	GENERATED_BODY()

protected:

	//~ Begin UPopupBase Interface
	virtual void Register() override;
	virtual void Unregister() override;
	//~ End UPopupBase Interface

	//~ Begin UCommonActivatableWidget Interface
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	//~ End UCommonActivatableWidget Interface

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
	// ~ End UI

private:

	// ~ Begin Button Callbacks
	void HandleCreateSessionClicked();
	void HandleQuickJoinClicked();
	void HandleJoinCodeClicked();
	void HandleJoinClicked();
	// ~ End Button Callbacks
};
