// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmLeaderBoardEntry.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"


void URhythmLeaderBoardEntry::UpdateData(int32 InRank, int32 InScore, bool bInIsLocalPlayer)
{
	Rank = InRank;
	Score = InScore;
	bIsLocalPlayer = bInIsLocalPlayer;

	if (RankText)
	{
		RankText->SetText(FText::Format(NSLOCTEXT("", "", "{0}등"), Rank));
	}

	if (ScoreText)
	{
		ScoreText->SetText(FText::Format(NSLOCTEXT("", "", "{0}점"), Score));
	}

	if (RightBackground)
	{
		if (bIsLocalPlayer && LocalPlayerBG)
		{
			RightBackground->SetBrushFromTexture(LocalPlayerBG, true);
		}
		else if (OtherPlayerBG)
		{
			RightBackground->SetBrushFromTexture(OtherPlayerBG, true);
		}
	}
}

void URhythmLeaderBoardEntry::NativeConstruct()
{
	Super::NativeConstruct();
	TargetRank = Rank;
	UpdateData(Rank, Score, bIsLocalPlayer);
}

void URhythmLeaderBoardEntry::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CanvasSlot || TargetRank <= 0)
	{
		return;
	}

	const float TargetY = (TargetRank - 1) * RowHeight;

	FVector2D Pos = CanvasSlot->GetPosition();
	Pos.Y = FMath::FInterpTo(Pos.Y, TargetY, InDeltaTime, MoveSpeed);

	CanvasSlot->SetPosition(Pos);
}
