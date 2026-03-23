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
	UFUNCTION()
	void HandleMatchmakingStarted();
	UFUNCTION()
	void HandleMatchmakingCompleted(FName SessionName, EEasyMatchmakingCompleteResult Result);
	UFUNCTION()
	void HandleMatchmakingCanceled();
};