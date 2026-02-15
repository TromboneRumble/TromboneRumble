// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OptionPanelBase.h"
#include "VideoOptionPanel.generated.h"

class UVerticalBox;
class UOptionCycleWidget;

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
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void Init(TFunction<void()> BackAction) override;
	
protected:
	virtual void HandleBackButtonClicked() override;
	virtual void HandleApplyButtonClicked() override;
	virtual void HandleResetButtonClicked() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UOptionCycleWidget> OptionCycleWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTable> GraphicsOptionsDataTable;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_OptionContainer;
	
	UPROPERTY()
	TMap<EGraphicsOptionType, UOptionCycleWidget*> CreatedWidgets;
	
	void BuildOptions();
	void UpdateUIFromEngineSettings();
	
	UFUNCTION()
	void OnOverallQualityChanged(int32 NewIndex);
	UFUNCTION()
	void OnSubOptionChanged(int32 NewIndex);
	UFUNCTION()
	void OnWindowModeChanged(int32 NewIndex);
	
private:
	float MinimumResolutionWidth = 1280.0f;
	float AspectRatio = 1.777f; // 16:9
};
