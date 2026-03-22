#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "FadeWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FFadeDelegate);

UCLASS()
class TROMBONERUMBLE_API UFadeWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	UFadeWidget();
	
public:
	
	/** Starts the fade-in animation */
	void StartFadeIn();
	
	/** Starts the fade-out animation */
	void StartFadeOut();
	
	/** Delegate called when fade-in starts */
	FFadeDelegate OnFadeInStarted;
	
	/** Delegate called when fade-in completes */
	FFadeDelegate OnFadeInComplete;
	
	/** Delegate called when fade-out starts */
	FFadeDelegate OnFadeOutStarted;
	
	/** Delegate called when fade-out completes */
	FFadeDelegate OnFadeOutComplete;
	
protected:
	
	/** Handles the completion of the fade-in animation */
	UFUNCTION()
	void HandleFadeInComplete();
	
	/** Handles the completion of the fade-out animation */
	UFUNCTION()
	void HandleFadeOutComplete();
	
protected:
	
	/** Fade animation */
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeAnimation;
	
protected:
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	// ~ End UCommonActivatableWidget Interface
};
