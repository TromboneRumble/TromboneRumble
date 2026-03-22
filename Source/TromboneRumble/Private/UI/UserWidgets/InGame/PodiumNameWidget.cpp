// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/InGame/PodiumNameWidget.h"
#include "Components/TextBlock.h"

void UPodiumNameWidget::SetPlayerName(const FString& InName)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(InName));
	}
}
