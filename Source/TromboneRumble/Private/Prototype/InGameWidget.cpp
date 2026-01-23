// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/InGameWidget.h"
#include "Components/Image.h"

void UInGameWidget::ToggleGuideUI()
{
	const ESlateVisibility CurrentVisibility = Image_Guide->GetVisibility();
	const ESlateVisibility TargetVisibility = CurrentVisibility == ESlateVisibility::Visible ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	
	Image_Guide->SetVisibility(TargetVisibility);
}
