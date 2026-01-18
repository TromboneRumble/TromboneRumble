// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "MainMenu/BaseMenuWidget.h"
#include "MatchMenuWidget.generated.h"

class UCommonTextBlock;
class UEasySessionSubsystem;
class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UMatchMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()
	
public:
	virtual void Init(TFunction<void()> OnMenuClosedCallback);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	
	virtual void Init() override;
	
	TFunction<void()> OnMenuClosed;
	
private:
	UFUNCTION()
	void HandleStartButtonClicked();

	virtual void SetUIEnabled(const bool bEnabled) override;
	
	// ~ Begin UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Start;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Back;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Code;
	// ~ End UIs
	
	UPROPERTY(Transient)
	FString CachedMainMenuMapPath = "";
	UPROPERTY(Transient)
	FString CachedLobbyMapPath = "";
};