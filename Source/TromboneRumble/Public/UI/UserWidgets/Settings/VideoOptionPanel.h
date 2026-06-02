#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "OptionPanelBase.h"
#include "VideoOptionPanel.generated.h"

class UVerticalBox;
class UOptionCycleRowWidget;

UENUM()
enum class EGraphicsOptionType : uint8
{
	OverallQuality,
	ViewDistance,
	AntiAliasing,
	PostProcess,
	Shadow,
	GlobalIllumination,
	Reflections,
	Texture,
	Effects,
	Resolution,
	VSync,
	WindowMode,
};

USTRUCT()
struct FGraphicsOptionRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGraphicsOptionType OptionType = EGraphicsOptionType::OverallQuality;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName = FText::FromString(TEXT("Option Name"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FText> OptionLabels = {
		FText::FromString(TEXT("Low")),
		FText::FromString(TEXT("Medium")),
		FText::FromString(TEXT("High")),
		FText::FromString(TEXT("Epic")),
		FText::FromString(TEXT("Cinematic"))
	};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DefaultIndex = 0;
};

UCLASS()
class TROMBONERUMBLE_API UVideoOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()
	
public:
	
	// ~ Begin UOptionPanelBase Interface
	virtual void RefreshUI() override;
	virtual void ApplySettingsFromUI(bool bSaveToDisk) override;
	virtual void ApplySettingsFromSavedData() override;
	virtual bool IsDirty() const override;
	// ~ End UOptionPanelBase Interface
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UOptionCycleRowWidget> OptionCycleWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTable> GraphicsOptionsDataTable;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_OptionContainer;
	
	UPROPERTY()
	TMap<EGraphicsOptionType, UOptionCycleRowWidget*> CreatedWidgets;

private:
	void BuildOptions();
	
	UFUNCTION()
	void OnOverallQualityChanged(int32 Value, ERotatorDirection RotatorDir);
	UFUNCTION()
	void OnSubOptionChanged(int32 Value, ERotatorDirection RotatorDir);
	UFUNCTION()
	void OnWindowModeChanged(int32 Value, ERotatorDirection RotatorDir);
	
public:
	
	// ~ Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	// ~ End UUserWidget Interface
	
};