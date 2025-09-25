// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/PT_UIInGame.h"
#include "Components/HorizontalBox.h"

void UPT_UIInGame::ShowInteractionHint(const bool bShow) const
{
	if (!InteractionHint) return;

	const ESlateVisibility IsVisible = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	InteractionHint->SetVisibility(IsVisible);
}

void UPT_UIInGame::NativeConstruct()
{
	Super::NativeConstruct();

	InteractionHint->SetVisibility(ESlateVisibility::Hidden);
}
