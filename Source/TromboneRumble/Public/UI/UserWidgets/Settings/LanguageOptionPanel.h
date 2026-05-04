#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Settings/OptionPanelBase.h"
#include "LanguageOptionPanel.generated.h"

enum class ERotatorDirection : uint8;
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
	
	UPROPERTY(EditAnywhere, Category = "Options")
	TArray<FString> SupportedCultures = { TEXT("ko"), TEXT("en") };

public:
	
	// ~ Begin UOptionPanelBase Interface
	virtual void RefreshUI() override;
	virtual void ApplySettingsFromUI(bool bSaveToDisk) override;
	virtual void ApplySettingsFromSavedData() override;
	virtual bool IsDirty() const override;
	// ~ End UOptionPanelBase Interface
	
protected:
	
	// ~ Begin UOptionPanelBase Interface
	virtual void Register() override;
	virtual void Unregister() override;
	// ~ End UOptionPanelBase Interface
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOptionCycleRowWidget> OC_Language;

private:
	
	UFUNCTION()
	void OnLanguageRotated(int32 Value, ERotatorDirection RotatorDir);
	
	
};
