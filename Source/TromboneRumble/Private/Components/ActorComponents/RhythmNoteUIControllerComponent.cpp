// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ActorComponents/RhythmNoteUIControllerComponent.h"
#include "TromboneGamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetBase.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetBase.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"


URhythmNoteUIControllerComponent::URhythmNoteUIControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URhythmNoteUIControllerComponent::InitSettings(URhythmSpawnWidgetBase* InSpawnWidget, URhythmNoteWidgetBase* InNoteWidget, const FNoteHandle& InHandle)
{
	RhythmSpawnWidget = InSpawnWidget;
	RhythmNoteWidget = InNoteWidget;
	Handle = InHandle;
	UnbindChannel();
	BindChannel();
}

void URhythmNoteUIControllerComponent::SpawnRhythmResultWidget(ENoteResult InResult)
{
	if (RhythmSpawnWidget.Get() && RhythmNoteWidget.Get())
	{
		FVector2D Pos = FVector2D::ZeroVector;

		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(RhythmNoteWidget->Slot))
		{
			Pos = Slot->GetPosition();
		}

		RhythmSpawnWidget->SpawnPooledRhythmResultWidget(Pos, InResult);
	}
}

void URhythmNoteUIControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	UnbindChannel();
	Super::EndPlay(EndPlayReason);
}

void URhythmNoteUIControllerComponent::BindChannel()
{

	if (URhythmNoteChannelSubsystem* RhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>())
	{
		if (FNoteChannel* Channel = RhythmNoteChannelSubsystem->GetChannelById(Handle.Id))
		{
			ProgressHandle = Channel->OnProgress.AddWeakLambda(this, [this](float Alpha)
				{
					UpdateNotePosition(Alpha);
				});
			DespawnHandle = Channel->OnDespawn.AddWeakLambda(this, [this]()
				{
					if (RhythmNoteWidget.IsValid() && RhythmSpawnWidget.IsValid()) {
						RhythmSpawnWidget->ReleasePooledRhythmNoteWidget(RhythmNoteWidget.Get());
						RhythmSpawnWidget = nullptr;
						RhythmNoteWidget = nullptr;
					}
				});
		}
	}
}

void URhythmNoteUIControllerComponent::UnbindChannel()
{
	if (URhythmNoteChannelSubsystem* RhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>())
	{
		if (FNoteChannel* Channel = RhythmNoteChannelSubsystem->GetChannelById(Handle.Id))
		{
			if (ProgressHandle.IsValid())
			{
				Channel->OnProgress.Remove(ProgressHandle);
				ProgressHandle.Reset();
			}
			if (DespawnHandle.IsValid())
			{
				Channel->OnDespawn.Remove(DespawnHandle);
				DespawnHandle.Reset();
			}
		}
	}
}

void URhythmNoteUIControllerComponent::UpdateNotePosition(const float InAlpha)
{
	if (RhythmNoteWidget.IsValid())
	{
		RhythmNoteWidget->UpdateNotePosition(InAlpha);
	}
}
