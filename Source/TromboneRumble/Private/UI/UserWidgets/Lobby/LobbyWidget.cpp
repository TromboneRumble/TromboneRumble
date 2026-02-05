// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Lobby/LobbyWidget.h"
#include "CommonTextBlock.h"
#include "Components/Button.h"
#include "Framework/LobbyGameState.h"
#include "Utilities/Defines.h"

bool ULobbyWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->OnLobbyStateChanged.AddDynamic(this, &ThisClass::OnLobbyStateUpdated);
	}

	return true;
}

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (CT_Countdown)
	{
		CT_Countdown->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ULobbyWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	Super::NativeDestruct();
}

void ULobbyWidget::OnLobbyStateUpdated(const ELobbyState NewState)
{
	if (!CT_Countdown) return;
	
	if (NewState == ELobbyState::CountdownToScramble || NewState == ELobbyState::CountdownToTravel)
	{
		InternalCountdownSeconds = CountdownSeconds;
		CT_Countdown->SetText(FText::AsNumber(InternalCountdownSeconds));
		CT_Countdown->SetVisibility(ESlateVisibility::Visible);
		GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ULobbyWidget::UpdateCountdown, 1.0f, true);
	}
	else
	{
		CT_Countdown->SetVisibility(ESlateVisibility::Hidden);
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
}

void ULobbyWidget::UpdateCountdown()
{
	if (!CT_Countdown) return;

	InternalCountdownSeconds--;
	CT_Countdown->SetText(FText::AsNumber(InternalCountdownSeconds));

	if (InternalCountdownSeconds <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
		CT_Countdown->SetVisibility(ESlateVisibility::Hidden);
	}
}