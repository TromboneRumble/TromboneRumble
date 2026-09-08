// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Lobby/LobbyWidget.h"
#include "CommonTextBlock.h"
#include "Components/Button.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Framework/LobbyGameState.h"
#include "Utilities/Defines.h"

void ULobbyWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Bound on activate and removed on deactivate. The stack reuses this instance
	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->OnLobbyStateChanged.AddDynamic(this, &ThisClass::OnLobbyStateUpdated);
	}

	if (CT_Countdown)
	{
		CT_Countdown->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ULobbyWidget::NativeOnDeactivated()
{
	if (UWorld* World = GetWorld())
	{
		if (ALobbyGameState* LobbyGameState = World->GetGameState<ALobbyGameState>())
		{
			LobbyGameState->OnLobbyStateChanged.RemoveDynamic(this, &ThisClass::OnLobbyStateUpdated);
		}
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::NativeOnDeactivated();
}

void ULobbyWidget::OnLobbyStateUpdated(const ELobbyState NewState)
{
	if (!CT_Countdown)
	{
		return;
	}

	int32 CountdownSeconds = 0;
	if (NewState == ELobbyState::CountdownToStandup)
	{
		CountdownSeconds = UTromboneConfig::Get()->LobbyRagdollGetUpDelaySeconds;
	}
	else if (NewState == ELobbyState::CountdownToTravel)
	{
		CountdownSeconds = UTromboneConfig::Get()->LobbyGameStartDelaySeconds;
	}

	if (CountdownSeconds > 0)
	{
		InternalCountdownSeconds = CountdownSeconds;
		
		CT_Countdown->SetText(FText::AsNumber(InternalCountdownSeconds));
		CT_Countdown->SetVisibility(ESlateVisibility::Visible);
		
		GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ThisClass::UpdateCountdown, 1.0f, true);
	}
	else
	{
		CT_Countdown->SetVisibility(ESlateVisibility::Hidden);
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
}

void ULobbyWidget::UpdateCountdown()
{
	if (!CT_Countdown)
	{
		return;
	}

	InternalCountdownSeconds--;
	CT_Countdown->SetText(FText::AsNumber(InternalCountdownSeconds));

	if (InternalCountdownSeconds <= 0)
	{
		CT_Countdown->SetVisibility(ESlateVisibility::Hidden);
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
}