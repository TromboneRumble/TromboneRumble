// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ActorComponents/RhythmNoteUIControllerComponent.h"
#include "TromboneGamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"

URhythmNoteUIControllerComponent::URhythmNoteUIControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URhythmNoteUIControllerComponent::SetStartPoses(const float InLaneStartXPos, const float InLaneEndXPos,
	const TArray<float>& InLaneYPosArray)
{
	LaneXStartPos = InLaneStartXPos;
	LaneXEndPos = InLaneEndXPos;
	LaneYPosArray = InLaneYPosArray;
}

void URhythmNoteUIControllerComponent::InitSettings(URhythmSpawnWidget* InSpawnWidget, URhythmNoteWidget* InNoteWidget,
                                                    int32 InLaneIndex)
{
	checkf(InSpawnWidget, TEXT("InSpawnWidget is nullptr"));
	checkf(InNoteWidget, TEXT("InNoteWidget is nullptr"));
	RhythmSpawnWidget = InSpawnWidget;
	RhythmNoteWidget = InNoteWidget;
	LaneIndex = InLaneIndex;
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

void URhythmNoteUIControllerComponent::OnViewportChanged(FGameplayTag Channel, const FViewportChangedMessage& InMsg)
{
	if (RhythmNoteWidget)
	{
		//const FVector2D ViewSize = InMsg.NewViewportSize;
		//const FVector2D StartNorm = FVector2D(LaneXStartPos / ViewSize.X, LaneYPosArray.IsValidIndex(LaneIndex) ? (LaneYPosArray[LaneIndex] / ViewSize.Y) : 0.f);
		//const FVector2D EndNorm = FVector2D(LaneXEndPos / ViewSize.X, LaneYPosArray.IsValidIndex(LaneIndex) ? (LaneYPosArray[LaneIndex] / ViewSize.Y) : 0.f);
		//RhythmNoteWidget->InitializeNote(StartNorm, EndNorm);
	}
}

