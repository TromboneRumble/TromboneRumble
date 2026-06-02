#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "BaseMenuWidget.generated.h"

class UBaseUIRoot;
class UEasyFriendSubsystem;
class UEasySessionSubsystem;

UCLASS()
class TROMBONERUMBLE_API UBaseMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, EMouseLockMode::LockOnCapture, false);
	}	

protected:
	virtual void NativeConstruct() override;
	
	virtual void Init();
	virtual void BindSubsystemCallbacks();
	virtual void RemoveSubsystemCallbacks();
	virtual void SetUIEnabled(const bool bEnabled);
	
	UFUNCTION()
	virtual void ShowLoadingOverlay();
	virtual void ShowLoadingOverlay(FString InContent);
	UFUNCTION()
	virtual void HideLoadingOverlay();
	
	TObjectPtr<UBaseUIRoot> GetRootLayout() const;
	
	UPROPERTY(Transient)
	TObjectPtr<UEasySessionSubsystem> SessionsSubsystem;
	
	UPROPERTY(Transient)
	TObjectPtr<UEasyFriendSubsystem> FriendsSubsystem;
};
