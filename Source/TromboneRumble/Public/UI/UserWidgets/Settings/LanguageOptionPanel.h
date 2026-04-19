#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Settings/OptionPanelBase.h"
#include "LanguageOptionPanel.generated.h"

class UOptionCycleWidget;

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
	
	virtual void RefreshUI() override;
	
protected:
	virtual void Activate() override;
	virtual void Deactivate() override;
	
	virtual void HandleApplyButtonClicked() override;
	virtual void HandleResetButtonClicked() override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOptionCycleWidget> OC_Language;
	
	UPROPERTY(EditAnywhere)
	TArray<FString> SupportedCultures;
	
private:
	int32 CurrentLanguageIndex;
	
};
