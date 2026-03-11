// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmLeaderBoardEntry.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"


void URhythmLeaderBoardEntry::UpdateData(const FLinearColor& InSkinColor, int32 InScore, bool bInIsLocalPlayer)
{
	Score = InScore;
	bIsLocalPlayer = bInIsLocalPlayer;
	SkinColor = InSkinColor;

	if (ScoreText)
	{
		FString PaddedScore = FString::Printf(TEXT("%06d"), Score);
		ScoreText->SetText(FText::FromString(PaddedScore));
	}

	if (PlayerBackGround)
	{
		PlayerBackGround->SetColorAndOpacity(InSkinColor);
	}
}

void URhythmLeaderBoardEntry::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateData(SkinColor, Score, bIsLocalPlayer);
}

void URhythmLeaderBoardEntry::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CanvasSlot)
	{
		return;
	}

	FVector2D Pos = CanvasSlot->GetPosition();
	Pos.Y = FMath::FInterpTo(Pos.Y, TargetY, InDeltaTime, MoveSpeed);
	CanvasSlot->SetPosition(Pos);
}
