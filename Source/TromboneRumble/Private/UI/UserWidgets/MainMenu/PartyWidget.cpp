#include "UI/UserWidgets/MainMenu/PartyWidget.h"
#include "EasyPartyManager.h"
#include "Beacons/EasyPartyPlayerState.h"
#include "Components/DynamicEntryBox.h"
#include "Engine/GameInstance.h"
#include "UI/UserWidgets/MainMenu/PartyPlayerWidget.h"

UPartyWidget::UPartyWidget()
{
	bCreateEntryForLocalPlayer = true;
}

void UPartyWidget::NativeOnInitialized()
{
	UEasyPartyManager* PartyManager = UEasyPartyManager::Get(this);
	PartyManager->OnDisconnectedFromParty().AddDynamic(this, &ThisClass::OnDisconnectedFromParty);
	PartyManager->OnPlayerStateAdded().AddDynamic(this, &ThisClass::OnPlayerStateAdded);
	PartyManager->OnPlayerStateRemoved().AddDynamic(this, &ThisClass::OnPlayerStateRemoved);

	// Create player widgets for existing party players.
	if (PartyManager->IsInParty())
	{
		for (AEasyPartyPlayerState* PartyPlayer : PartyManager->GetPartyPlayerStates())
		{
			CreatePlayerWidget(PartyPlayer);
		}
	}

	Super::NativeOnInitialized();
}

void UPartyWidget::CreatePlayerWidget(AEasyPartyPlayerState* OwningPlayerState)
{
	if (PartyPlayerEntryBox)
	{
		const bool bIsLocalPlayer = GetGameInstance()->GetPrimaryPlayerUniqueIdRepl() == OwningPlayerState->UniqueId;
		if (bIsLocalPlayer ? bCreateEntryForLocalPlayer : true)
		{
			if (UPartyPlayerWidget* PlayerWidget = PartyPlayerEntryBox->CreateEntry<UPartyPlayerWidget>())
			{
				PlayerWidget->InitPlayerWidget(OwningPlayerState);
			}
		}
	}
}

void UPartyWidget::RemovePlayerWidget(const FUniqueNetIdRepl& PlayerId)
{
	if (PartyPlayerEntryBox)
	{
		for (UPartyPlayerWidget* PlayerWidget : PartyPlayerEntryBox->GetTypedEntries<UPartyPlayerWidget>())
		{
			if (PlayerWidget->GetPlayerUniqueId() == PlayerId)
			{
				PartyPlayerEntryBox->RemoveEntry(PlayerWidget);
				return;
			}
		}
	}
}

void UPartyWidget::OnDisconnectedFromParty()
{
	if (PartyPlayerEntryBox) PartyPlayerEntryBox->Reset(true);
}

void UPartyWidget::OnPlayerStateAdded(AEasyPartyPlayerState* PlayerState)
{
	CreatePlayerWidget(PlayerState);
}

void UPartyWidget::OnPlayerStateRemoved(AEasyPartyPlayerState* PlayerState)
{
	RemovePlayerWidget(PlayerState->UniqueId);
}