// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/UIInGame.h"
#include "Components/HorizontalBox.h"

void UUIInGame::ShowInteractionHint(const bool bShow) const
{
	if (!InteractionHint) return;

	const ESlateVisibility IsVisible = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	InteractionHint->SetVisibility(IsVisible);
}

void UUIInGame::NativeConstruct()
{
	Super::NativeConstruct();

	InteractionHint->SetVisibility(ESlateVisibility::Hidden);
}
