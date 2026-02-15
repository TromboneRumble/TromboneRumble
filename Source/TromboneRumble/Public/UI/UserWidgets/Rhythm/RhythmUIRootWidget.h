// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Iris/Core/IrisProfiler.h"
#include "RhythmUIRootWidget.generated.h"

enum class ERhythmGameState : uint8;
enum class ENoteResult : uint8;
class ADefaultPlayerState;
enum class EScoreType : uint8;
class UProgressBar;
class UTextBlock;
class URhythmLeaderBoard;
enum class EInstrumentType : uint8;
class URhythmSpawnWidgetBase;
class UCanvasPanel;
class URhythmSpawnWidget;

UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmUIRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void OnGameEnded();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void UpdateScoreText(APlayerState* AffectedPlayerState, int32 AddedAmount, EScoreType ScoreType);

	UFUNCTION()
	void UpdateComboText(ENoteResult InNoteResult, int32 ComboCount);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URhythmLeaderBoard> WBP_LeaderBoard;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> MusicProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ComboText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ComboNumberText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNamedSlot> AddedScoreNameSlot;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ShowLeaderboardAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ScoreUpdatedAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ComboHitAnim;

private:
	UFUNCTION()
	void HandleRhythmGameStateChanged(ERhythmGameState RhythmGameState);

	UFUNCTION()
	void OnPlayerStateChanged(APlayerState* NewPlayerState);

	void BindDelegates(ADefaultPlayerState* InDefaultPlayerState);

	void UpdateProgressbar(float DeltaSeconds);
	bool hasGameStarted = false;
	bool IsSongPaused = false;
	float CurrentSongTotalLength = 0.f;
	float CurrentTime = 0.f;
	int32 CurrentSongPlayingID = 0;

};