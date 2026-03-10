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
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	bool bIsLocalPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FLinearColor SkinColor = FLinearColor::White;

	void UpdateData(const FLinearColor& InSkinColor, int32 InScore, bool bInIsLocalPlayer);
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerBackGround;
private:
	float TargetY = 0.f;
	float RowHeight = 50.f;
	float MoveSpeed = 10.f;

	
public:
	FORCEINLINE void SetTargetY(float InTargetY) { TargetY = InTargetY; }
	FORCEINLINE void SetRowHeight(float InRowHeight) { RowHeight = InRowHeight; }
};
