// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PopupBase.h"
#include "JoinCodePopup.generated.h"

class UEditableText;

UCLASS()
class TROMBONERUMBLE_API UJoinCodePopup : public UPopupBase
{
	GENERATED_BODY()
	
protected:
	
	// ~ Begin UPopupWidgetBase Interface
	virtual void Register() override;
	virtual void Unregister() override;
	virtual UWidget* GetDefaultFocusWidget() const override;
	// ~ End UPopupWidgetBase Interface
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_JoinCode;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_Code;
	
private:
	
	void OnClickJoinCode();
	
};
