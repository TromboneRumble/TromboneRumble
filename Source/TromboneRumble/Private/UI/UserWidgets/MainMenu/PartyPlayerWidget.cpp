#include "UI/UserWidgets/MainMenu/PartyPlayerWidget.h"
#include "CommonTextBlock.h"
#include "Beacons/EasyPartyPlayerState.h"

UPartyPlayerWidget::UPartyPlayerWidget()
{
	OwningPlayerState = nullptr;
}

void UPartyPlayerWidget::InitPlayerWidget(AEasyPartyPlayerState* InOwningPlayerState)
{
	if (InOwningPlayerState)
	{
		OwningPlayerState = InOwningPlayerState;

		OwningPlayerState->OnPartyOwnerChanged().AddUObject(this, &ThisClass::OnPartyLeaderChanged);

		const FText Name = GetPlayerName();
		CT_Name->SetText(Name);
		
		// Make sure that we didn't miss a party owner change.
		// Checking after widget initialization, so that during initialization we can just simply hide the party leader icon, and wait for this delegate to trigger.
		if (OwningPlayerState->PartyOwnerUniqueId.IsValid())
		{
			OnPartyLeaderChanged(OwningPlayerState->PartyOwnerUniqueId);
		}
	}
	
	else UE_LOG(LogTemp, Error, TEXT("EasyPartyPlayerWidget: Failed to initialize!"));
}

FUniqueNetIdRepl& UPartyPlayerWidget::GetPlayerUniqueId() const
{
	if (OwningPlayerState)
	{
		return OwningPlayerState->UniqueId;
	}

	UE_LOG(LogTemp, Error, TEXT("PartyPlayerWidget: GetPlayerUniqueId() called on player widget with no OwningPlayerState."));
	static FUniqueNetIdRepl EmptyId = FUniqueNetIdRepl();
	return EmptyId;
}

FText UPartyPlayerWidget::GetPlayerName() const
{
	if (OwningPlayerState)
	{
		return OwningPlayerState->DisplayName;
	}

	UE_LOG(LogTemp, Error, TEXT("PartyPlayerWidget: GetPlayerName() called on player widget with no OwningPlayerState."));
	return FText::GetEmpty();
}

bool UPartyPlayerWidget::IsPartyLeader() const
{
	if (OwningPlayerState)
	{
		return OwningPlayerState->IsPartyLeader();
	}

	UE_LOG(LogTemp, Error, TEXT("PartyPlayerWidget: IsPartyLeader() called on player widget with no OwningPlayerState."));
	return false;
}

void UPartyPlayerWidget::OnPartyLeaderChanged(const FUniqueNetIdRepl& UniqueId)
{
	if (OwningPlayerState->PartyOwnerUniqueId.IsValid())
	{
		const bool bIsLeader = IsPartyLeader();
		CT_Leader->SetVisibility(bIsLeader ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}