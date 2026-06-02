// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "Utilities/Defines.h"
#include "MatchMenuWidget.generated.h"

enum class EEasyMatchmakingCompleteResult : uint8;
enum class EEasyMatchmakingState : uint8;
class UTromboneGameInstance;
enum class ERotatorDirection : uint8;
class UCommonRotatorWidgetBase;
enum class EMatchType : uint8;
class UCommonRotator;
class UCommonTextBlock;
class UEasySessionSubsystem;
class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UMatchMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()

public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::NoCapture, EMouseLockMode::LockOnCapture, false);
	}

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnInitialized() override;

	virtual void Init() override;
	virtual void SetUIEnabled(const bool bEnabled) override;
	
protected:
	
	// ~ Begin UIs
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonButtonBase> CB_Start;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonButtonBase> CB_Back;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonTextBlock> CT_Code;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonRotatorWidgetBase> CR_MatchType;
	// ~ End UIs
	
private:
	// ~ Begin GameState Events
	void BindGameStateEvents();
	void RemoveGameStateEvents();
	
	UFUNCTION()
	void OnMatchTypeChanged(EMatchType NewType);
	// ~ End GameState Events
	
	// ~ Begin UI Events
	UFUNCTION()
	void HandleStartButtonClicked();
	UFUNCTION()
	void HandleBackButtonClicked();
	UFUNCTION()
	void HandleOnRotatedMatchType(int32 Value, ERotatorDirection RotatorDir);
	// ~ End UI Events
	
	UPROPERTY(Transient)
	FString CachedMainMenuMapPath = "";
	UPROPERTY(Transient)
	FString CachedLobbyMapPath = "";
	
	bool bIsStarted = false;
	
	UFUNCTION()
	void HandleMatchmakingUpdated(const EEasyMatchmakingState MatchmakingState, const int32 MatchmakingTime);
	UFUNCTION()
	void HandleOnUpdateCompleteInMatchmaking(bool bWasSuccessful);
};