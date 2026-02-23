// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "OptionCycleWidget.generated.h"

class UCommonTextBlock;
class UCommonRotator;
class UCommonButtonBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionChanged, int32, NewIndex);

UCLASS()
class TROMBONERUMBLE_API UOptionCycleWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void Init(FText InName, TArray<FText> InOptions, int32 DefaultIndex);
	virtual void SetIsEnabled(bool bInIsEnabled) override;
	
	int32 GetCurrentIndex() const;
	void SetSelectedIndex(int32 NewIndex);
	const TArray<FText>& GetOptionsArray() const { return OptionsArray; }
	
	FOnOptionChanged OnOptionChanged;
	
protected:
	virtual void InitButtons();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonRotator> WBP_OptionRotator;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Prev;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Next;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_OptionName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FText> OptionsArray = {
		FText::FromString(TEXT("Low")),
		FText::FromString(TEXT("Medium")),
		FText::FromString(TEXT("High")),
		FText::FromString(TEXT("Epic")),
		FText::FromString(TEXT("Cinematic"))
	};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DefaultSelectedIndex = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TextOptionName = FText::FromString(TEXT("Option Name"));
};
