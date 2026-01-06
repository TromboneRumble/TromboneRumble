// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/VideoOptionPanel.h"
#include "CommonTextBlock.h"
#include "Components/VerticalBox.h"
#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleWidget.h"

void UVideoOptionPanel::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	Text_OptionPanelTitle->SetText(FText::FromString(TEXT("비디오 옵션")));
	BuildOptions();
}

void UVideoOptionPanel::Init(TFunction<void()> BackAction)
{
	Super::Init(BackAction);
}

void UVideoOptionPanel::HandleBackButtonClicked()
{
	Super::HandleBackButtonClicked();
}

void UVideoOptionPanel::HandleApplyButtonClicked()
{
	Super::HandleApplyButtonClicked();
}

void UVideoOptionPanel::HandleResetButtonClicked()
{
	Super::HandleResetButtonClicked();
}

void UVideoOptionPanel::BuildOptions()
{
	if (!GraphicsOptionsDataTable || !OptionCycleWidgetClass || !VB_OptionContainer) return;

	VB_OptionContainer->ClearChildren();
	CreatedWidgets.Empty();

	static const FString ContextString(TEXT("Graphics Option Context"));
	TArray<FGraphicsOptionRow*> AllRows;
	GraphicsOptionsDataTable->GetAllRows<FGraphicsOptionRow>(ContextString, AllRows);

	for (const FGraphicsOptionRow* Row : AllRows)
	{
		if (Row)
		{
			if (UOptionCycleWidget* NewWidget = CreateWidget<UOptionCycleWidget>(this, OptionCycleWidgetClass))
			{
				NewWidget->Init(Row->DisplayName, Row->OptionLabels, Row->DefaultIndex);
				VB_OptionContainer->AddChild(NewWidget);
				CreatedWidgets.Add(Row->OptionType, NewWidget);
			}
		}
	}
}