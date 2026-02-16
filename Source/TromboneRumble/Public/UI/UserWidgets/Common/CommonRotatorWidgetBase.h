// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "CommonRotatorWidgetBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionChanged, int32, NewIndex);

UCLASS()
class TROMBONERUMBLE_API UCommonRotatorWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;
	virtual void NativePreConstruct() override;
	virtual void Init(TArray<FText> InOptions, int32 InDefaultIndex);
	virtual void SetIsEnabled(bool bInIsEnabled) override;
	
	FOnOptionChanged OnOptionChanged;
	FOnRotatedWithDirection& OnRotatedWithDirection() const { return CR_Rotator->OnRotatedWithDirection; }

protected:
	virtual void InitButtons();	
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonRotator> CR_Rotator;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Prev;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Next;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FText> OptionsArray;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DefaultSelectedIndex = 0;
	
public:
	// ~ Begin Getter & Setter
	int32 GetCurrentIndex() const { return CR_Rotator ? CR_Rotator->GetSelectedIndex() : -1; }
	void SetSelectedIndex(int32 NewIndex);
	const TArray<FText>& GetOptionsArray() const { return OptionsArray; }
	// ~ End Getter & Setter
};
