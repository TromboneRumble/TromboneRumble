#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Settings/OptionPanelBase.h"
#include "LanguageOptionPanel.generated.h"

class UOptionCycleRowWidget;

UENUM()
enum class ELanguageType : uint8
{
	Korean = 0,
	English = 1,
};

UCLASS()
class TROMBONERUMBLE_API ULanguageOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	ULanguageOptionPanel();

protected:
	
	// ~ Begin UOptionPanelBase Interface
	virtual void RefreshUI() override;
	virtual void ReapplySavedSettings() override;
	virtual void HandleApplyButtonClicked() override;
	virtual void HandleResetButtonClicked() override;
	// ~ End UOptionPanelBase Interface
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOptionCycleRowWidget> OC_Language;
	
	UPROPERTY(EditAnywhere)
	TArray<FString> SupportedCultures;
	
private:
	
	int32 CurrentLanguageIndex;
	
};
