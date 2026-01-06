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
	Resolution,
	AntiAliasing,
	Shadow,
	Texture,
	PostProcess,
	VSync
};

USTRUCT()
struct FGraphicsOptionRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGraphicsOptionType OptionType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FText> OptionLabels;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DefaultIndex = 0;
};

UCLASS()
class TROMBONERUMBLE_API UVideoOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
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
};
