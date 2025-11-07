// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ActorComponents/RhythmNoteUIControllerComponent.h"
#include "TromboneGamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmNoteWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

URhythmNoteUIControllerComponent::URhythmNoteUIControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URhythmNoteUIControllerComponent::InitSettings(URhythmNoteWidget* InNoteWidget, const FNoteHandle& InHandle, const int32 InLineIdx)
{
	
	RhythmNoteWidget = InNoteWidget;
	Handle = InHandle;
	LaneIndex = InLineIdx;
	BindChannel();
}

void URhythmNoteUIControllerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (UWorld* World = GetWorld())
	{
		UGameplayMessageSubsystem& Msg = UGameplayMessageSubsystem::Get(World);

		LayoutChangedHandle = Msg.RegisterListener<FViewportChangedMessage>(
			TromboneGamePlayTags::Trombone_Rhythm_OnLayoutChanged.GetTag(),
			this,
			&URhythmNoteUIControllerComponent::OnViewportChanged
		);
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
					if (RhythmNoteWidget.IsValid()) RhythmNoteWidget->RemoveFromParent();
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
			if (ProgressHandle.IsValid()) Channel->OnProgress.Remove(ProgressHandle);
			if (DespawnHandle.IsValid())  Channel->OnDespawn.Remove(DespawnHandle);
		}
	}
}

void URhythmNoteUIControllerComponent::OnViewportChanged(FGameplayTag Channel, const FViewportChangedMessage& InMsg)
{
	SetStartPoses(InMsg.LaneXStartPos, InMsg.LaneXEndPos, InMsg.LaneYPosArray);
}

void URhythmNoteUIControllerComponent::SetStartPoses(const float InLaneStartXPos, const float InLaneEndXPos,
	const TArray<float>& InLaneYPosArray)
{
	LaneXStartPos = InLaneStartXPos;
	LaneXEndPos = InLaneEndXPos;
	LaneYPosArray = InLaneYPosArray;
}

void URhythmNoteUIControllerComponent::UpdateNotePosition(const float InAlphaOnSpline)
{
	if (RhythmNoteWidget.IsValid())
	{
		const float XPos = FMath::Lerp(LaneXStartPos, LaneXEndPos, InAlphaOnSpline);
		const float YPos = LaneYPosArray.IsValidIndex(LaneIndex) ? LaneYPosArray[LaneIndex] : 0.f;
		if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(RhythmNoteWidget->Slot))
		{
			CanvasPanelSlot->SetPosition(FVector2D(XPos, YPos));
		}
	}
}
