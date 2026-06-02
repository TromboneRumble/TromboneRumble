// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Iris/Core/IrisProfiler.h"
#include "RhythmUIRootWidget.generated.h"


enum class EInGameState : uint8;
class UProgressBar;
class UTextBlock;
class UCanvasPanel;

enum class EScoreType : uint8;
enum class ERhythmGameState : uint8;
enum class ENoteResult : uint8;
enum class EInstrumentType : uint8;

class URhythmTimeWidget;
class URhythmLeaderBoard;
class URhythmFloatingScoreWidget;
class ADefaultPlayerState;

UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmUIRootWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Components
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URhythmLeaderBoard> WBP_LeaderBoard;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<URhythmTimeWidget> WBP_RhythmTimeWidget;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> AddedScoreContainer;
	// ~Components

	// Animations

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ScoreUpdatedAnim;
	// ~Animations

	UPROPERTY(EditDefaultsOnly, Category = "Rhythm")
	TSubclassOf<URhythmFloatingScoreWidget> FloatingScoreWidgetClass;

private:
	void BindDelegates();
	FTimerHandle TimerHandle_RetryBind;
	void RetryBindDelegates();

	UFUNCTION()
	void HandleOnPlayerStateChanged(APlayerState* NewPlayerState);


	UFUNCTION()
	void HandleOnLocalScoreChanged(APlayerState* PlayerState, int32 AddedAmount, EScoreType ScoreType);

	UFUNCTION()
	void UpdateScoreText(APlayerState* AffectedPlayerState, int32 AddedAmount, EScoreType ScoreType);

	UFUNCTION()
	void SpawnFloatingScoreWidget(APlayerState* AffectedPlayerState, int32 AddedAmount, EScoreType ScoreType);

};