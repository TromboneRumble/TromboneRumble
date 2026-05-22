#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "MainMenuWidget.generated.h"

enum class EEasyMatchmakingCompleteResult : uint8;
class UCommonButtonBase;
class UEditableText;

UCLASS()
class TROMBONERUMBLE_API UMainMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	virtual void Init() override;
	virtual void SetUIEnabled(const bool bEnabled) override;
	
	virtual void BindSubsystemCallbacks() override;
	virtual void RemoveSubsystemCallbacks() override;
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> SettingPopupClass;
	
	// ~ Begin UI
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_Code;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_CreateSession;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_QuickJoin;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Join;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Settings;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Tutorial;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Quit;
	// ~ End UI
	
	UPROPERTY(Transient)
	FString CachedMatchMenuMapPath = "";
	
private:
	// ~ Begin Button Callbacks
	UFUNCTION()
	void HandleCreateSessionClicked();
	UFUNCTION()
	void HandleQuickJoinButtonClicked();
	UFUNCTION()
	void HandleJoinButtonClicked();
	UFUNCTION()
	void HandleTutorialButtonClicked();
	// ~ End Button Callbacks
	
	/** Displays the tutorial popup */
	void ShowTutorialPopup();
	
	/** Displays the quit confirmation popup */
	void ShowQuitPopup();
	
private:
	
	/** Called when matchmaking starts. */
	UFUNCTION()
	void HandleMatchmakingStarted();
	
	/** Called when matchmaking is complete. */
	UFUNCTION()
	void HandleMatchmakingComplete(const FName SessionName, const EEasyMatchmakingCompleteResult Result);
	
	/** Called when matchmaking is canceled. */
	UFUNCTION()
	void HandleMatchmakingCanceled();
	
};