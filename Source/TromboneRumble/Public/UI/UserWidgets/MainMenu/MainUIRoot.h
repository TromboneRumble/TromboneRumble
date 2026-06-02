#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "MainUIRoot.generated.h"

enum class EEasyMatchmakingCompleteResult : uint8;
class UCommonActivatableWidget;

UCLASS()
class TROMBONERUMBLE_API UMainUIRoot : public UBaseUIRoot
{
	GENERATED_BODY()
	
protected:
	virtual void Register() override;

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