// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/CommonRotatorWidgetBase.h"
#include "OptionCycleRowWidget.generated.h"

class UCommonTextBlock;
class UCommonRotator;
class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UOptionCycleRowWidget : public UCommonRotatorWidgetBase
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, Category = "Options");
	FText OptionRowTitleText = FText::GetEmpty();

public:
	
	virtual void ForceInit(FText InName, TArray<FText> InOptions, int32 DefaultIndex);

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_OptionName;
	
public:
	
	// ~ Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	// ~ End UUserWidget Interface
};
