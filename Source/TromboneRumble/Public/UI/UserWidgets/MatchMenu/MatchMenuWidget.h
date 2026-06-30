// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "Utilities/Defines.h"
#include "MatchMenuWidget.generated.h"

class AMatchMenuGameState;
class UCommonButtonBaseExtensionWithText;
class UCommonRotatorWidgetBase;
enum class EEasyMatchmakingState : uint8;
enum class ERotatorDirection : uint8;
enum class EMatchType : uint8;

UCLASS()
class TROMBONERUMBLE_API UMatchMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()

public:
	
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::NoCapture, EMouseLockMode::LockOnCapture, false);
	}
	
	// ~ Begin UBaseMenuWidget Interface
	virtual void Init() override;
	virtual void SetUIEnabled(const bool bEnabled) override;
	// ~ End UBaseMenuWidget Interface
	
protected:
	
	// ~ Begin UIs
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonButtonBaseExtensionWithText> CB_Start;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonButtonBaseExtensionWithText> CB_Back;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonRotatorWidgetBase> CR_MatchType;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonRotatorWidgetBase> CR_Map;
	// ~ End UIs
	
private:
	
	// ~ Begin GameState Events
	UFUNCTION()
	void OnMatchTypeChanged(EMatchType NewType);
	UFUNCTION()
	void OnSelectedMapChanged(FGameplayTag NewMapTag);
	// ~ End GameState Events
	
	// ~ Begin UI Events
	UFUNCTION()
	void HandleStartButtonClicked();
	UFUNCTION()
	void HandleBackButtonClicked();
	UFUNCTION()
	void HandleOnRotatedMatchType(int32 Value, ERotatorDirection RotatorDir);
	UFUNCTION()
	void HandleOnRotatedMap(int32 Value, ERotatorDirection RotatorDir);
	// ~ End UI Events

	void InitSelectableMaps();
	
	UFUNCTION()
	void HandleOnUpdateMatchComplete(bool bWasSuccessful);
	
private:
	
	bool bIsStarted = false;
	
	UPROPERTY(Transient)
	TArray<FGameplayTag> CachedSelectableMaps;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<AMatchMenuGameState> CachedMatchMenuGS;
	
protected:
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// End UCommonActivatableWidget Interface
};