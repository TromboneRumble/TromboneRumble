// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/CommonRotatorWidgetBase.h"
#include "OptionCycleWidget.generated.h"

class UCommonTextBlock;
class UCommonRotator;
class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UOptionCycleWidget : public UCommonRotatorWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	// TODO : 부모 클래스의 Init을 숨김. 수정해야 함
	virtual void Init(FText InName, TArray<FText> InOptions, int32 DefaultIndex);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_OptionName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TextOptionName = FText::FromString(TEXT("Option Name"));
};
