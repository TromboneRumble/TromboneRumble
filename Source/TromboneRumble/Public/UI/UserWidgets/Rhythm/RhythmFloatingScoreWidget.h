// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmFloatingScoreWidget.generated.h"

enum class EScoreType : uint8;
class UTextBlock;
class UImage;
class UTexture2D;


USTRUCT(BlueprintType)
struct FScoreVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rhythm")
	UTexture2D* IconTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rhythm")
	FLinearColor TextColor = FLinearColor::White;
};

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API URhythmFloatingScoreWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	void Init(int32 ScoreAmount, EScoreType ScoreType);

protected:
	virtual void NativeConstruct() override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ScoreIcon;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FloatAndFadeAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Rhythm|Score")
	TMap<EScoreType, FScoreVisualData> ScoreVisuals;
};
