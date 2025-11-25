// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmLeaderBoardEntry.generated.h"

class UImage;
class UTextBlock;

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmLeaderBoardEntry : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	int32 Rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	bool bIsLocalPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FLinearColor SkinColor = FLinearColor::White;

	void UpdateData(const FLinearColor& InSkinColor, int32 InRank, int32 InScore, bool bInIsLocalPlayer);
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RankText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerBackGround;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RightBackground;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leaderboard|Style")
	TObjectPtr<UTexture2D> LocalPlayerBG;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leaderboard|Style")
	TObjectPtr<UTexture2D> OtherPlayerBG;
private:
	int32 TargetRank = 0;
	float RowHeight = 50.f;
	float MoveSpeed = 10.f;

	
public:
	FORCEINLINE void SetTargetRank(int32 InTargetRank) { TargetRank = InTargetRank; }
	FORCEINLINE void SetRowHeight(float InRowHeight) { RowHeight = InRowHeight; }
};
