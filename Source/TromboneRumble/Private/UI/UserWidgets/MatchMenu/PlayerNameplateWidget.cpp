#include "UI/UserWidgets/MatchMenu/PlayerNameplateWidget.h"
#include "Framework/DefaultPlayerState.h"
#include "Utilities/DebugHelper.h"

UPlayerNameplateWidget::UPlayerNameplateWidget()
{
	OwningPlayerState = nullptr;
}

void UPlayerNameplateWidget::InitPlayerWidget(ADefaultPlayerState* InOwningPlayerState)
{
	if (InOwningPlayerState)
	{
		OwningPlayerState = InOwningPlayerState;
		
		OwningPlayerState->OnPlayerNameChanged.AddDynamic(this, &ThisClass::OnPlayerNameChanged);
		
		const FString PlayerName = OwningPlayerState->GetPlayerName();
		if (!PlayerName.IsEmpty())
		{
			OnPlayerNameChanged(PlayerName);
		}
		return;
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Invalid player state provided to player nameplate widget."));
}

void UPlayerNameplateWidget::OnPlayerNameChanged(const FString& PlayerName)
{
	K2_OnPlayerNameChanged(PlayerName);
}

bool UPlayerNameplateWidget::IsLocallyControlledPlayer() const
{
	if (OwningPlayerState)
	{
		if (const APlayerController* PC = GetOwningPlayer())
		{
			return PC->PlayerState == OwningPlayerState;
		}
	}
	
	return false;
}

bool UPlayerNameplateWidget::IsHostPlayer() const
{
	return OwningPlayerState ? OwningPlayerState->IsHost() : false;
}
