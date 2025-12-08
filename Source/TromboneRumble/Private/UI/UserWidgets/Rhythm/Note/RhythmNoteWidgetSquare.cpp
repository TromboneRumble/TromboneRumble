// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetSquare.h"
#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetSquare.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "Components/CanvasPanelSlot.h"
#include "Utilities/Defines.h"

void URhythmNoteWidgetSquare::Init(URhythmSpawnWidgetBase* InOwner)
{
	Super::Init(InOwner);
}

void URhythmNoteWidgetSquare::InitWithCueMessage(const FString& InUserCueName)
{
	FString LastChar = InUserCueName.Right(1);
	LaneIndex = FCString::Atoi(*LastChar);
}

void URhythmNoteWidgetSquare::UpdateNotePosition(const float InAlpha)
{
	const float XPos = FMath::Lerp(LaneXStartPos, LaneXEndPos, InAlpha);
	const float YPos = LaneYPosArray.IsValidIndex(LaneIndex) ? LaneYPosArray[LaneIndex] : 0.f;
	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasPanelSlot->SetPosition(FVector2D(XPos, YPos));
	}
}

void URhythmNoteWidgetSquare::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!IsDesignTime())
	{
		if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UGameplayMessageSubsystem* Msg = GameInstance->GetSubsystem<UGameplayMessageSubsystem>())
			{
				LayoutChangedHandle = Msg->RegisterListener<FViewportChangedMessage>(
					TromboneGamePlayTags::Trombone_Rhythm_OnLayoutChanged.GetTag(),
					this,
					&ThisClass::OnViewportChanged
				);
			}
		}
	}
}


void URhythmNoteWidgetSquare::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UGameplayMessageSubsystem* Msg = GameInstance->GetSubsystem<UGameplayMessageSubsystem>())
		{
			if (LayoutChangedHandle.IsValid())
			{
				Msg->UnregisterListener(LayoutChangedHandle);
				LayoutChangedHandle = {};
			}
		}
	}
	Super::NativeDestruct();
}

void URhythmNoteWidgetSquare::OnViewportChanged(FGameplayTag Channel, const FViewportChangedMessage& InMsg)
{
	LaneXStartPos = InMsg.LaneXStartPos;
	LaneXEndPos = InMsg.LaneXEndPos;
	LaneYPosArray = InMsg.LaneYPosArray;
}
